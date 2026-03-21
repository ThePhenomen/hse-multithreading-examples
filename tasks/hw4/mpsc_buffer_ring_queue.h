#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <atomic>
#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <span>
#include <vector> 

#include "../../include/utils/error.hpp"

constexpr int PROTOCOL_VERSION = 1;

struct MsgHeader {
    int type{};
    int length{};
};

struct QueueMeta {
    int version{PROTOCOL_VERSION};
    int capacity{};
    std::atomic<int> head_idx{0};
    std::atomic<int> tail_idx{0};
    std::atomic<int> used_idx{0};
};

inline void write_ring(std::byte* buffer, int capacity, int offset, const std::byte* data, int size) {
    int first_part = std::min(size, capacity - (offset % capacity));
    std::memcpy(buffer + (offset % capacity), data, first_part);
    if (first_part < size) {
        std::memcpy(buffer, data + first_part, size - first_part);
    }
}

inline void read_ring(const std::byte* buffer, int capacity, int offset, std::byte* output, int size) {
    int first_part = std::min(size, capacity - (offset % capacity));
    std::memcpy(output, buffer + (offset % capacity), first_part);
    if (first_part < size) {
        std::memcpy(output + first_part, buffer, size - first_part);
    }
}

class ProducerNode {
public:
    ProducerNode(const std::string& shared_file, int capacity) : queue_capacity(capacity) {
        size_t total_size = sizeof(QueueMeta) + capacity;

        int fd = shm_open(shared_file.c_str(), O_CREAT | O_RDWR, 0666);
        CHECK_ERROR(fd);
        CHECK_ERROR(ftruncate(fd, total_size));
        void* ptr = mmap(nullptr, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (ptr == MAP_FAILED) {
            throw std::runtime_error{std::format("mmap failed, error: {}", strerror(errno))};
        }
        close(fd);

        queue_meta = new (ptr) QueueMeta{};
        queue_meta->version = PROTOCOL_VERSION;
        queue_meta->capacity = capacity;
        queue_buffer = reinterpret_cast<std::byte*>(queue_meta + 1);
    }

    bool push(int type, std::span<const std::byte> msg) {
        int msg_size = msg.size();
        int required_space = sizeof(MsgHeader) + msg_size;

        if (required_space > queue_capacity) {
            return false;
        }

        int head;
        while (true) {
            head = queue_meta->head_idx.load();
            auto tail = queue_meta->tail_idx.load();

            if (head + required_space - tail > queue_capacity) {
                return false;
            }

            if (queue_meta->head_idx.compare_exchange_weak( head, head + required_space)) {
                break;
            }
        }

        MsgHeader header{type, msg_size};
        write_ring(queue_buffer, queue_capacity, head, reinterpret_cast<const std::byte*>(&header), sizeof(MsgHeader));
        write_ring(queue_buffer, queue_capacity, head + sizeof(MsgHeader), msg.data(), msg_size);

        int used = head;
        while (!queue_meta->used_idx.compare_exchange_weak(used, head + required_space)) {
            used = head;
        }

        return true;
    }

private:
    QueueMeta* queue_meta{};
    std::byte* queue_buffer{};
    int queue_capacity{};
};

class ConsumerNode {
public:
    ConsumerNode(const std::string& shared_file) {
        int fd = shm_open(shared_file.c_str(), O_RDWR, 0666);
        CHECK_ERROR(fd);
        void* head_ptr = mmap(nullptr, sizeof(QueueMeta), PROT_READ, MAP_SHARED, fd, 0);
        if (head_ptr == MAP_FAILED) {
            throw std::runtime_error{std::format("mmap header failed, error: {}", strerror(errno))};
        }

        auto* init_meta = static_cast<QueueMeta*>(head_ptr);
        if (init_meta->version != PROTOCOL_VERSION) {
            throw std::runtime_error("Protocol version mismatch!");
        }
        queue_capacity = init_meta->capacity;
        size_t total_size = sizeof(QueueMeta) + queue_capacity;

        void* ptr = mmap(nullptr, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (ptr == MAP_FAILED) {
            throw std::runtime_error{std::format("mmap full failed, error: {}", strerror(errno))};
        }
        close(fd);

        queue_meta = static_cast<QueueMeta*>(ptr);
        queue_buffer = reinterpret_cast<std::byte*>(queue_meta + 1);
    }

    std::optional<std::vector<std::byte>> pop(int type) {
        while (true) {
            std::optional<std::vector<std::byte>> result;

            auto tail = queue_meta->tail_idx.load();
            auto used = queue_meta->used_idx.load();

            if (tail == used) {
                return result;
            }

            MsgHeader header;
            read_ring(queue_buffer, queue_capacity, tail, reinterpret_cast<std::byte*>(&header), sizeof(MsgHeader));
            int total_msg_size = sizeof(MsgHeader) + header.length;
            if (tail + total_msg_size > used) {
                return result;
            }

            bool is_correct_type = (header.type == type);
            queue_meta->tail_idx.store(tail + total_msg_size);

            if (is_correct_type) {
                result = std::vector<std::byte>(header.length);
                read_ring(queue_buffer, queue_capacity, tail + sizeof(MsgHeader), result->data(), header.length);
                return result;
            }
        }
    }

private:
    QueueMeta* queue_meta{};
    std::byte* queue_buffer{};
    int queue_capacity{};
};
