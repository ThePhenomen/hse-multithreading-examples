#include "thread_pool.h"
#include <iostream>
#include <stdexcept>

int Worker(int id) {
    if (id == 228) {
        throw std::runtime_error("Invalid ID provided");
    }
    return id * 10;
}

int main() {
    std::cout << "Starting ThreadPool" << std::endl;
    ThreadPool pool(2);

    auto valid_case = pool.Submit(Worker, 10);
    auto invalid_case = pool.Submit(Worker, 228);

    try {
        std::cout << "Valid Result: " << valid_case.get() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Unexpected error: " << e.what() << std::endl;
    }

    try {
        int res = invalid_case.get();
        std::cout << "IF PRINTED THAN ERROR IN ThreadPool: invalid_case result = " << res << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception caught successfully: " << e.what() << std::endl;
    }

    std::cout << "Passed" << std::endl;
    return 0;
}
