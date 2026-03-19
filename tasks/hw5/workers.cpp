#include "thread_pool.h"
#include <iostream>
#include <chrono>

int ComputeMultiply(int a, int b) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return a * b;
}

int main() {
    std::cout << "Starting ThreadPool" << std::endl;
    ThreadPool pool(4);

    auto future1 = pool.Submit(ComputeMultiply, 10, 5);
    auto future2 = pool.Submit(ComputeMultiply, 20, 3);
    auto future3 = pool.Submit(ComputeMultiply, 100, 2);

    std::cout << "Waiting for results" << std::endl;
    std::cout << "Result 1: " << future1.get() << std::endl;
    std::cout << "Result 2: " << future2.get() << std::endl;
    std::cout << "Result 3: " << future3.get() << std::endl;
    
    std::cout << "Passed" << std::endl;
    return 0;
}
