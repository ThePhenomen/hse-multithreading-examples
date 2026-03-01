# Данный код собирался на отдельной ВМ на Linux, поскольку на MacOS не запустить код с futex 
CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Werror -O3 -pthread

TEST_BIN = futex_condvar_test

.PHONY: all test clean

all: $(TEST_BIN)

$(TEST_BIN): test.cpp futex_cond_var.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) test.cpp -o $(TEST_BIN) $(LDFLAGS) -lgtest -lgtest_main

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(TEST_BIN)
