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

        sender_waiter.wait(lock, [this]() { return !is_writing || is_closed; });
        if (is_closed) {
            throw std::runtime_error("Channel is closed");
        }

        is_writing = true;
        buffer = value;
        receiver_notifier.notify_one();

        writer_waiter.wait(lock, [this]() { return !buffer.has_value() || is_closed; });
        if (is_closed && buffer.has_value()) {
            buffer.reset();
            is_writing = false;
            sender_waiter.notify_all();
            throw std::runtime_error("Channel is closed");
        }

        is_writing = false;
        sender_waiter.notify_one();
    }

    std::optional<T> Recv() {
        std::unique_lock<std::mutex> lock(mtx);

        receiver_notifier.wait(lock, [this]() { return buffer.has_value() || is_closed; });
        if (!buffer.has_value()) {
            return std::nullopt;
        }

        T result = std::move(*buffer);
        buffer.reset();
        writer_waiter.notify_one();

        return result;
    }

    void Close() {
        std::unique_lock<std::mutex> lock(mtx);
        if (!is_closed) {
            is_closed = true;
            sender_waiter.notify_all();
            receiver_notifier.notify_all();
            writer_waiter.notify_all();
        }
    }

private:
    std::mutex mtx;
    std::condition_variable sender_waiter;
    std::condition_variable receiver_notifier;
    std::condition_variable writer_waiter;
    bool is_writing = false;
    bool is_closed = false;

    std::optional<T> buffer;
};
