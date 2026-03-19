CXX = clang++
CXXFLAGS = -std=c++23 -Wall -Wextra -Werror -O3 -pthread

INCLUDE_DIR = .

TEST_BASIC_BIN = run_test
TEST_EXP_BIN = run_exp_test

.PHONY: all clean test

all: $(TEST_BASIC_BIN) $(TEST_EXP_BIN)

$(TEST_BASIC_BIN): workers.cpp thread_pool.h
	$(CXX) -I$(INCLUDE_DIR) $(CXXFLAGS) workers.cpp -o $(TEST_BASIC_BIN)

$(TEST_EXP_BIN): failed_worker.cpp thread_pool.h
	$(CXX) -I$(INCLUDE_DIR) $(CXXFLAGS) failed_worker.cpp -o $(TEST_EXP_BIN)

test: all
	@echo "==============================="
	@echo "Running basic test"
	@echo "==============================="
	./$(TEST_BASIC_BIN)
	@echo "==============================="
	@echo "Running basic test"
	@echo "==============================="
	./$(TEST_EXP_BIN)
	@echo "==============================="
	@echo "Tests finished"
	@echo "==============================="

clean:
	rm -f $(TEST_BASIC_BIN) $(TEST_EXP_BIN)
