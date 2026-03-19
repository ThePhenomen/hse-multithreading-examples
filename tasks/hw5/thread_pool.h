#pragma once

#include <condition_variable>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

template <typename T>
struct InternalFutexState {
    std::mutex mtx;
    std::condition_variable cv;
    std::exception_ptr exception = nullptr;

    std::optional<T> value;
};

template <typename T>
class Future {
public:
    Future(std::shared_ptr<InternalFutexState<T>> state) : internal_state(std::move(state)) {}

    T get() {
        std::unique_lock lock(internal_state->mtx);
        internal_state->cv.wait(lock, [this]() { return internal_state->value.has_value() || internal_state->exception != nullptr; });

        if (internal_state->exception) {
            std::rethrow_exception(internal_state->exception);
        }
        return std::move(*(internal_state->value));
    }

private:
    std::shared_ptr<InternalFutexState<T>> internal_state;
};

template <typename T>
class Promise {
public:
    Promise() : internal_state(std::make_shared<InternalFutexState<T>>()) {}

    Future<T> get_future() {
        return Future<T>(internal_state);
    }

    void set_value(T value) {
        {
            std::unique_lock lock(internal_state->mtx);
            internal_state->value = std::move(value);
        }
        internal_state->cv.notify_all();
    }

    void set_exception(std::exception_ptr e) {
        {
            std::unique_lock lock(internal_state->mtx);
            internal_state->exception = std::move(e);
        }
        internal_state->cv.notify_all();
    }

private:
    std::shared_ptr<InternalFutexState<T>> internal_state;
};

class ThreadPool {
public:
    // 16 CPU kernels
    ThreadPool(size_t num_threads = 16) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(mtx);
                        state_change.wait(lock, [this]() { return stop || !tasks.empty(); });

                        if (stop && tasks.empty()) {
                            return;
                        }

                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock lock(mtx);
            stop = true;
        }
        state_change.notify_all();
    }

    template <typename F, typename... Args>
    auto Submit(F f, Args... args) {
        using declared_type = decltype(f(args...));
        auto promise = std::make_shared<Promise<declared_type>>();
        auto future = promise->get_future();

        auto task = [prm = promise, func = std::bind(f, args...)]() mutable {
            try {
                prm->set_value(func());
            } catch (...) {
                prm->set_exception(std::current_exception());
            }
        };

        {
            std::unique_lock lock(mtx);
            tasks.push(task);
        }
        state_change.notify_one();

        return future;
    }

private:
    std::mutex mtx;
    std::condition_variable state_change;
    bool stop = false;

    std::vector<std::jthread> workers;
    std::queue<std::function<void()>> tasks;
};
