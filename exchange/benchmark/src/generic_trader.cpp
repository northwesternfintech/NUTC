#include "helpers/benchmark_trader.hpp"

#include <benchmark/benchmark.h>

using namespace nutc;

static void
BM_ModifyHoldings(benchmark::State& state)
{
    benchmarks::BenchmarkTrader trader;
    uint32_t adds = static_cast<uint32_t>(state.range(0));

    for (auto _ : state) {
        for (uint32_t i = 0; i < adds; i++) {
            trader.get_portfolio().modify_holdings(common::Ticker::ETH, 1.0);
            trader.get_portfolio().modify_holdings(common::Ticker::BTC, 1.0);
            trader.get_portfolio().modify_holdings(common::Ticker::LTC, 1.0);
        }
    }
}

BENCHMARK(BM_ModifyHoldings)->Range(128, 128 << 10);
