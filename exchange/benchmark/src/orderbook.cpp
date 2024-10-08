#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "helpers/benchmark_trader.hpp"

#include <benchmark/benchmark.h>

using namespace nutc;

static void
BM_AddOrders(benchmark::State& state)
{
    exchange::LimitOrderBook orderbook;
    benchmarks::BenchmarkTrader trader;
    uint32_t adds = static_cast<uint32_t>(state.range(0));

    for (auto _ : state) {
        for (uint32_t i = 0; i < adds; i++) {
            orderbook.add_order(
                {trader, common::Ticker::ETH, common::Side::sell, 0.0, 0.0, false}
            );
        }
    }
}

BENCHMARK(BM_AddOrders)->Range(128, 128 << 10);
