CXX = clang++
CXXFLAGS = -std=c++23 -Wall -Wextra -Werror -O3 -pthread

INCLUDE_DIR = ../include

PRODUCER_BIN = producer
CONSUMER_BIN = consumer

.PHONY: all clean test

all: $(PRODUCER_BIN) $(CONSUMER_BIN)

$(PRODUCER_BIN): producer.cpp mpsc_buffer_ring_queue.h
	$(CXX) -I$(INCLUDE_DIR) $(CXXFLAGS) producer.cpp -o $(PRODUCER_BIN)

$(CONSUMER_BIN): consumer.cpp mpsc_buffer_ring_queue.h
	$(CXX) -I$(INCLUDE_DIR) $(CXXFLAGS) consumer.cpp -o $(CONSUMER_BIN)

test: all
	@echo "Starting producer in background"; \
	./$(PRODUCER_BIN) & \
	PID_PROD=$$!; \
	sleep 1; \
	echo "Starting consumer"; \
	./$(CONSUMER_BIN); \
	echo "Consumer done. Waiting for producer to exit"; \
	wait $$PID_PROD; \
	echo "Test finished"

clean:
	rm -f $(PRODUCER_BIN) $(CONSUMER_BIN)
