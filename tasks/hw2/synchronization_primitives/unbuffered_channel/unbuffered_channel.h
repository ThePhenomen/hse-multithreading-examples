#pragma once

#include <optional>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

template <class T>
class UnbufferedChannel {
public:
    UnbufferedChannel() = default;

    void Send(const T& value) {
        std::unique_lock<std::mutex> lock(mtx);

        sender_notifier.wait(lock, [this]() { return !is_writing || is_closed; });
        if (is_closed) {
            throw std::runtime_error("Channel is closed");
        }

        is_writing = true;
        buffer = value;
        is_empty = false;
        receiver_notifier.notify_one();

        reader_notifier.wait(lock, [this]() { return is_empty || is_closed; });
        if (is_closed && !is_empty) {
            is_empty = true;
            buffer.reset();
            is_writing = false;
            sender_notifier.notify_all();
            throw std::runtime_error("Channel is closed");
        }

        is_writing = false;
        sender_notifier.notify_one();
    }

    std::optional<T> Recv() {
        std::unique_lock<std::mutex> lock(mtx);

        receiver_notifier.wait(lock, [this]() { return !is_empty || is_closed; });
        if (is_empty) {
            return std::nullopt;
        }

        T result = std::move(buffer.value());
        buffer.reset();
        is_empty = true;
        reader_notifier.notify_one();

        return result;
    }

    void Close() {
        std::unique_lock<std::mutex> lock(mtx);
        if (!is_closed) {
            is_closed = true;
            sender_notifier.notify_all();
            receiver_notifier.notify_all();
            reader_notifier.notify_all();
        }
    }

private:
    std::mutex mtx;
    std::condition_variable sender_notifier;
    std::condition_variable receiver_notifier;
    std::condition_variable reader_notifier;
    bool is_writing = false;
    bool is_empty = true;
    bool is_closed = false;

    std::optional<T> buffer;
};
