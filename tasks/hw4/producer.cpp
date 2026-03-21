#include "mpsc_buffer_ring_queue.h"
#include <thread>
#include <chrono>

static std::span<const std::byte> ToBytes(const std::string& s) {
    return {
        reinterpret_cast<const std::byte*>(s.data()),
        s.size()
    };
}

int main() {
    static constexpr auto shared_file = "/tmp/mpsc_buffer_ring_queue";
    shm_unlink(shared_file);

    try {
        ProducerNode producer(shared_file, 1024);

        std::string msg1 = "First msg";
        if (producer.push(1, ToBytes(msg1))) {
            std::cout << std::format("[PRODUCER]: Push №1: {}", msg1) << std::endl;
        }

        std::string msg2 = "Second msg";
        if (producer.push(2, ToBytes(msg2))) {
            std::cout << std::format("[PRODUCER]: Push №2: {}", msg2) << std::endl;
        }

        std::string msg3 = "Third msg";
        if (producer.push(1, ToBytes(msg3))) {
            std::cout << std::format("[PRODUCER]: Push №3: {}", msg3) << std::endl;
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
