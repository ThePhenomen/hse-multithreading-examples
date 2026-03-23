#include "futex_cond_var.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>

TEST(FutexCondVarTest, NotifyOne) {
    FutexCondVar cv;
    std::mutex mtx;
    bool ready = false;

    std::thread waiter([&]() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return ready; });
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    cv.notify_one();

    waiter.join();
    EXPECT_TRUE(ready);
}

TEST(FutexCondVarTest, NotifyAll) {
    FutexCondVar cv;
    std::mutex mtx;
    bool ready = false;

    const int thread_count = 5;
    std::vector<std::thread> waiters;
    for (int i = 0; i < thread_count; ++i) {
        waiters.emplace_back([&]() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() { return ready; });
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    cv.notify_all();

    for (auto& waiter : waiters) {
        waiter.join();
    }
    EXPECT_TRUE(ready);
}

TEST(FutexCondVarTest, WaitForTimeout) {
    FutexCondVar cv;
    std::mutex mtx;
    bool ready = false;

    auto start_time = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> lock(mtx);
    bool result = cv.wait_for(lock, std::chrono::milliseconds(100), [&]() { return ready; });
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    EXPECT_FALSE(result);
    EXPECT_GE(elapsed, 80);
    EXPECT_LE(elapsed, 150);
}

TEST(FutexCondVarTest, WaitForResult) {
    FutexCondVar cv;
    std::mutex mtx;
    bool ready = false;

    std::thread worker([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        {
            std::lock_guard<std::mutex> lock(mtx);
            ready = true;
        }
        cv.notify_one();
    });

    std::unique_lock<std::mutex> lock(mtx);
    bool result = cv.wait_for(lock, std::chrono::milliseconds(500), [&]() { return ready; });

    worker.join();
    EXPECT_TRUE(result);
}

TEST(FutexCondVarTest, WaitUntilTimeout) {
    FutexCondVar cv;
    std::mutex mtx;

    std::unique_lock<std::mutex> lock(mtx);
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(50);
    std::cv_status status = cv.wait_until(lock, deadline);

    EXPECT_EQ(status, std::cv_status::timeout);
}

TEST(FutexCondVarTest, WaitUntilResult) {
    FutexCondVar cv;
    std::mutex mtx;
    bool ready = false;

    std::thread worker([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        {
            std::lock_guard<std::mutex> lock(mtx);
            ready = true;
        }
        cv.notify_one();
    });

    std::unique_lock<std::mutex> lock(mtx);
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    bool result = cv.wait_until(lock, deadline, [&]() { return ready; });

    worker.join();
    EXPECT_TRUE(result);
}