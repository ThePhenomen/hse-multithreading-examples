CXX      = clang++
CXXFLAGS = -std=c++23 -Wall -Wextra -Werror -O3 -pthread

INCLUDES = -I/opt/homebrew/include
LDFLAGS  = -L/opt/homebrew/lib

TEST_BIN  = unbuffered_channel_test
BENCH_BIN = unbuffered_channel_benchmark

.PHONY: all test bench clean

all: $(TEST_BIN) $(BENCH_BIN)

$(TEST_BIN): test.cpp unbuffered_channel.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) test.cpp -o $(TEST_BIN) $(LDFLAGS) -lgtest -lgtest_main

$(BENCH_BIN): benchmark.cpp unbuffered_channel.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) benchmark.cpp -o $(BENCH_BIN) $(LDFLAGS) -lbenchmark

test: $(TEST_BIN)
	./$(TEST_BIN)

bench: $(BENCH_BIN)
	./$(BENCH_BIN)

clean:
	rm -f $(TEST_BIN) $(BENCH_BIN)
