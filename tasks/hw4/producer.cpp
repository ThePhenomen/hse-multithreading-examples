#include "mpsc_buffer_ring_queue.h"
#include <thread>
#include <chrono>

int main() {
    static constexpr auto shared_file = "/tmp/mpsc_buffer_ring_queue";
    shm_unlink(shared_file);

    try {
        ProducerNode producer(shared_file, 10);

        std::string msg1 = "First msg";
        std::span<const std::byte> msg_bytes1{
            reinterpret_cast<const std::byte*>(msg1.data()),
            msg1.size()
        };
        if (producer.push(1, msg_bytes1)) {
            std::cout << std::format("[PRODUCER]: Push №1: {}", msg1) << std::endl;
        }

        std::string msg2 = "Second msg";
        std::span<const std::byte> msg_bytes2{
            reinterpret_cast<const std::byte*>(msg2.data()),
            msg2.size()
        };
        if (producer.push(2, msg_bytes2)) {
            std::cout << std::format("[PRODUCER]: Push №2: {}", msg2) << std::endl;
        }

        std::string msg3 = "Third msg";
        std::span<const std::byte> msg_bytes3{
            reinterpret_cast<const std::byte*>(msg3.data()),
            msg3.size()
        };
        if (producer.push(1, msg_bytes3)) {
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
