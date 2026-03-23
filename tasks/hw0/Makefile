CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -O3 -pthread

TEST_BIN = run_test

.PHONY: all clean test

all: $(TEST_BIN)

$(TEST_BIN): worker.cpp green_thread_scheduler.h
	$(CXX) -lboost_context worker.cpp -o $(TEST_BIN) $(LDFLAGS)

test: all
	@echo "==============================="
	@echo "Running test"
	@echo "==============================="
	./$(TEST_BIN)
	@echo "==============================="
	@echo "Test finished"
	@echo "==============================="

clean:
	rm -f $(TEST_BIN)
