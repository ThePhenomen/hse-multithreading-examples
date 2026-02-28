#include <benchmark/benchmark.h>
#include <cmath>
#include "apply_function.h"

// однопоточная версия стабильно быстрее многопоточной
static void BM_FastComputation_SingleThread(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<int> data(100, 1);
        state.ResumeTiming();
        
        ApplyFunction<int>(data, [](int& x) { x += 1; }, 1);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_FastComputation_SingleThread);

static void BM_FastComputation_MultiThread(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<int> data(100, 1);
        state.ResumeTiming();
        
        ApplyFunction<int>(data, [](int& x) { x += 1; }, 4);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_FastComputation_MultiThread);

// многопоточноя версия стабильно быстрее однопоточной
static void BM_HeavyComputation_SingleThread(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<double> data(1000000, 1.0);
        state.ResumeTiming();
        
        ApplyFunction<double>(data, [](double& x) { 
            for (int i = 0; i < 100; ++i) {
                x = pow(std::sin(x), 2) + pow(std::cos(x), 2);
            }
        }, 1);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_HeavyComputation_SingleThread);

static void BM_HeavyComputation_MultiThread(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<double> data(1000000, 2.0);
        state.ResumeTiming();
        
        int cores = 8;
        ApplyFunction<double>(data, [](double& x) { 
            for (int i = 0; i < 100; ++i) {
                x = pow(std::sin(x), 2) + pow(std::cos(x), 2);
            }
        }, cores); 
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_HeavyComputation_MultiThread);

BENCHMARK_MAIN();
