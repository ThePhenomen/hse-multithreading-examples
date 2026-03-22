CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -O3 -pthread

INCLUDES = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib

TEST_BIN = run_tests
BENCH_BIN = run_benchmarks

.PHONY: all clean test bench

all: $(TEST_BIN) $(BENCH_BIN)

$(TEST_BIN): test.cpp apply_function.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) test.cpp -o $(TEST_BIN) $(LDFLAGS) -lgtest -lgtest_main

$(BENCH_BIN): benchmark.cpp apply_function.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) benchmark.cpp -o $(BENCH_BIN) $(LDFLAGS) -lbenchmark

clean:
	rm -f $(TEST_BIN) $(BENCH_BIN)
