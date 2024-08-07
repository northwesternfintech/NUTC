#include "helpers/benchmark_trader.hpp"

#include <benchmark/benchmark.h>

using namespace nutc;

static void
BM_ModifyHoldings(benchmark::State& state)
{
    bench_utils::BenchmarkTrader trader;
    uint32_t adds = static_cast<uint32_t>(state.range(0));

    for (auto _ : state) {
        for (uint32_t i = 0; i < adds; i++) {
            trader.modify_holdings("ABC", 1);
            trader.modify_holdings("DEF", 1);
            trader.modify_holdings("HIJ", 1);
            trader.modify_holdings("KLM", 1);
        }
    }
}

BENCHMARK(BM_ModifyHoldings)->Range(128, 128 << 10);
