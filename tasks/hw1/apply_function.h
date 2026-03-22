#include <vector>
#include <functional>
#include <thread>
#include <algorithm>
#pragma once

template <typename T>
void ApplyFunction(std::vector<T>& data, const std::function<void(T&)>& transform, const int threadCount = 1) {
    if (data.empty()) return;

    int actualThreadCount = std::max(1, threadCount);
    if (static_cast<size_t>(actualThreadCount) > data.size()) {
        actualThreadCount = static_cast<int>(data.size());
    }

    auto worker = [&transform](std::vector<T>::iterator start, std::vector<T>::iterator end) {
        for (auto it = start; it != end; ++it) {
            transform(*it);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(actualThreadCount);
    size_t chunkSize = data.size() / actualThreadCount;
    size_t remains = data.size() % actualThreadCount;

    auto current = data.begin();
    for (size_t i = 0; i < static_cast<size_t>(actualThreadCount); ++i) {
        size_t currentChunkSize = chunkSize + (i < remains ? 1 : 0);
        auto next = current + currentChunkSize;
        
        threads.emplace_back(worker, current, next);
        current = next;
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
