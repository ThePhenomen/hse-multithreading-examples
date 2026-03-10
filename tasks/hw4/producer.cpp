#include "mpsc_buffer_ring_queue.h"
#include <thread>
#include <chrono>

int main() {
    static constexpr auto shared_file = "/tmp/mpsc_buffer_ring_queue";
    shm_unlink(shared_file);

    try {
        ProducerNode producer(shared_file, 10);

        std::string msg = "First msg";
        if (producer.push(1, msg)) {
            std::cout << std::format("[PRODUCER]: Push №1: {}", msg) << std::endl;
        }

        msg = "Second msg";
        if (producer.push(2, msg)) {
            std::cout << std::format("[PRODUCER]: Push №2: {}", msg) << std::endl;
        }

        msg = "Third msg";
        if (producer.push(1, msg)) {
            std::cout << std::format("[PRODUCER]: Push №3: {}", msg) << std::endl;
        }

        std::cout << "[PRODUCER]: Waiting for consumer" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));

    } catch (const std::exception& e) {
        std::cerr << std::format("[PRODUCER]: Error: {}", e.what()) << std::endl;
    }

    CHECK_ERROR(shm_unlink(shared_file));
    std::cout << "[PRODUCER]: Finished" << std::endl;
    return 0;
}
