#include <gtest/gtest.h>
#include "apply_function.h"

TEST(ApplyFunctionTest, SingleThread) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    ApplyFunction<int>(data, [](int& x) { x *= 2; }, 1);
    EXPECT_EQ(data, std::vector<int>({2, 4, 6, 8, 10}));
}

TEST(ApplyFunctionTest, MultiThread) {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8};
    ApplyFunction<int>(data, [](int& x) { x += 1; }, 3);
    EXPECT_EQ(data, std::vector<int>({2, 3, 4, 5, 6, 7, 8, 9}));
}

TEST(ApplyFunctionTest, ThreadCountGreaterThanDataSize) {
    std::vector<int> data = {1, 2, 3};
    ApplyFunction<int>(data, [](int& x) { x *= 10; }, 10);
    EXPECT_EQ(data, std::vector<int>({10, 20, 30}));
}

TEST(ApplyFunctionTest, ThreadCountZero) {
    std::vector<int> data = {1, 2, 3};
    ApplyFunction<int>(data, [](int& x) { x -= 1; },0);
    EXPECT_EQ(data, std::vector<int>({0, 1, 2}));
}

TEST(ApplyFunctionTest, ThreadCountNegative) {
    std::vector<int> data = {1, 2, 3};
    ApplyFunction<int>(data, [](int& x) { x += 1; }, -5);
    EXPECT_EQ(data, std::vector<int>({2, 3, 4}));
}

TEST(ApplyFunctionTest, EmptyVector) {
    std::vector<int> data;
    ApplyFunction<int>(data, [](int& x) { x *= 2; }, 4);
    EXPECT_TRUE(data.empty());
}
