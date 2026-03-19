#include "mpsc_buffer_ring_queue.h"
#include <thread>
#include <chrono>

int main() {
    static constexpr auto shared_file = "/tmp/mpsc_buffer_ring_queue";

    try {
        ConsumerNode consumer(shared_file);
        std::cout << "[CONSUMER]: Connected to shared memory ring buffer" << std::endl;

        size_t consumed = 0;
        while (consumed < 2) {
            auto result = consumer.pop(1);
            if (result) {
                std::string s(reinterpret_cast<const char*>(result->data()), result->size());
                std::cout << std::format("[CONSUMER]: Got msg: {}", s, result->size()) << std::endl;
                consumed++;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << std::format("[CONSUMER]: Error: {}", e.what()) << std::endl;
    }

    std::cout << "[CONSUMER]: Finished" << std::endl;
    return 0;
}
