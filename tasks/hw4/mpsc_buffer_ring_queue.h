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

#include "../../include/utils/error.hpp"

constexpr int PROTOCOL_VERSION = 1;
constexpr size_t MAX_PAYLOAD_SIZE = 256;

struct MsgHeader {
    int type{};
    int length{};
};

struct Element {
    std::atomic<bool> ready{false};
    MsgHeader header{};
    char payload[MAX_PAYLOAD_SIZE]{};
};

struct QueueMeta {
    int version{PROTOCOL_VERSION};
    int capacity{};
    std::atomic<int> head_idx{0};
    std::atomic<int> tail_idx{0};
};

class ProducerNode {
public:
    ProducerNode(const std::string& shared_file, int capacity) : queue_capacity(capacity) {
        size_t total_size = sizeof(QueueMeta) + capacity * sizeof(Element);

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
        queue_elements = reinterpret_cast<Element*>(queue_meta + 1);
        for (int i = 0; i < capacity; ++i) {
            new (&queue_elements[i]) Element{};
        }
    }

    bool push(int type, const std::string& msg) {
        if (msg.size() > MAX_PAYLOAD_SIZE) 
            return false;

        while (true) {
            auto head = queue_meta->head_idx.load();
            auto tail = queue_meta->tail_idx.load();

            if (head - tail >= queue_capacity) {
                return false;
            }

            if (queue_meta->head_idx.compare_exchange_weak(head, head + 1)) {
                Element& element = queue_elements[head % queue_capacity];
                element.header.type = type;
                element.header.length = msg.size();
                std::memcpy(element.payload, msg.data(), msg.size());
                element.ready.store(true);
                return true;
            }
        }
    }

private:
    QueueMeta* queue_meta{};
    Element* queue_elements{};
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
        size_t total_size = sizeof(QueueMeta) + queue_capacity * sizeof(Element);

        void* ptr = mmap(nullptr, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (ptr == MAP_FAILED) {
            throw std::runtime_error{std::format("mmap full failed, error: {}", strerror(errno))};
        }
        close(fd);

        queue_meta = static_cast<QueueMeta*>(ptr);
        queue_elements = reinterpret_cast<Element*>(queue_meta + 1);
    }

    std::optional<std::string> pop(int type) {
        while (true) {
            std::optional<std::string> result;

            auto tail = queue_meta->tail_idx.load();
            auto head = queue_meta->head_idx.load();
            if (tail == head) {
                return result;
            }

            Element& element = queue_elements[tail % queue_capacity];
            if (!element.ready.load()) {
                return std::nullopt;
            }

            bool is_correct_type = (element.header.type == type);
            element.ready.store(false);
            queue_meta->tail_idx.store(tail + 1);

            if (is_correct_type)
                result = std::string(element.payload, element.header.length);
                return result;
        }
    }

private:
    QueueMeta* queue_meta{};
    Element* queue_elements{};
    int queue_capacity{};
};
