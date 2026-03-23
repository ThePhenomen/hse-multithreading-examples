#pragma once

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <atomic>
#include <mutex>
#include <limits>
#include <chrono>
#include <cerrno>
#include <condition_variable>

long FutexWait(void* addr, int expected_val, const struct timespec* timeout=nullptr) {
    return syscall(SYS_futex, addr, FUTEX_WAIT_PRIVATE, expected_val, timeout, nullptr, 0);
}

void FutexWake(void* addr, int count) {
    syscall(SYS_futex, addr, FUTEX_WAKE_PRIVATE, count, nullptr, nullptr, 0);
}

class FutexCondVar {
public:
    FutexCondVar() = default;

    void wait(std::unique_lock<std::mutex>& lock) {
        const int expected = counter.load(std::memory_order_relaxed);
        lock.unlock();
        FutexWait(&counter, expected);

        lock.lock();
    }

    template <typename Predicate>
    void wait(std::unique_lock<std::mutex>& lock, Predicate pred) {
        while (!pred()) {
            wait(lock);
        }
    }

    template<class Clock, class Duration>
    std::cv_status wait_until(std::unique_lock<std::mutex>& lock, const std::chrono::time_point<Clock, Duration>& abs_time) {
        const int expected = counter.load(std::memory_order_relaxed);
        lock.unlock();

        auto now = Clock::now();
        if (now >= abs_time) {
            lock.lock();
            return std::cv_status::timeout;
        }
        auto left = abs_time - now;

        struct timespec ts;
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(left);
        auto nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(left - secs);
        ts.tv_sec = static_cast<time_t>(secs.count());
        ts.tv_nsec = static_cast<long>(nsecs.count());
        long res = FutexWait(&counter, expected, &ts);
        int err = (res == -1) ? errno : 0;
        lock.lock();
        if (err == ETIMEDOUT) {
            return std::cv_status::timeout;
        }
        return std::cv_status::no_timeout;
    }

    template<class Clock, class Duration, class Predicate>
    bool wait_until(std::unique_lock<std::mutex>& lock, const std::chrono::time_point<Clock, Duration>& abs_time, Predicate pred) {
        while (!pred()) {
            if (wait_until(lock, abs_time) == std::cv_status::timeout) {
                return pred();
            }
        }
        return true;
    }

    template<class Rep, class Period>
    std::cv_status wait_for(std::unique_lock<std::mutex>& lock, const std::chrono::duration<Rep, Period>& rel_time) {
        return wait_until(lock, std::chrono::steady_clock::now() + rel_time);
    }

    template<class Rep, class Period, class Predicate>
    bool wait_for(std::unique_lock<std::mutex>& lock, const std::chrono::duration<Rep, Period>& rel_time, Predicate pred) {
        return wait_until(lock, std::chrono::steady_clock::now() + rel_time, std::move(pred));
    }

    void notify_one() {
        counter.fetch_add(1, std::memory_order_relaxed);
        FutexWake(&counter, 1);
    }

    void notify_all() {
        counter.fetch_add(1, std::memory_order_relaxed);
        FutexWake(&counter, std::numeric_limits<int>::max());
    }

private:
    std::atomic<int> counter{0};
};
