#include "exchange/orders/level_tracking/level_quantity_tracker.hpp"

#include "common/types/decimal.hpp"

#include <benchmark/benchmark.h>

#include <random>

using namespace nutc;

static void
BM_AddRangeOfTimes(benchmark::State& state)
{
    exchange::LevelQuantityTracker tracker;
    uint16_t max_level = static_cast<uint16_t>(state.range(0));
    common::decimal_price decimal_price;
    common::decimal_quantity decimal_quantity;

    for (auto _ : state) {
        for (uint16_t level = 0; level < max_level; level++) {
            decimal_price.set_underlying(level);
            decimal_quantity.set_underlying(level);
            tracker.report_quantity(
                common::Side::sell, decimal_quantity, decimal_price
            );
        }
        for (uint16_t level = 0; level < max_level; level++) {
            decimal_price.set_underlying(level);
            benchmark::DoNotOptimize(
                tracker.get_level(common::Side::sell, decimal_price)
            );
        }
    }
}

BENCHMARK(BM_AddRangeOfTimes)->Range(128, 128 << 9);

static void
BM_RandomIterate(benchmark::State& state)
{
    exchange::LevelQuantityTracker tracker;
    uint32_t max_level = static_cast<uint32_t>(state.range(0));

    std::vector<uint16_t> random_levels(max_level);
    std::iota(random_levels.begin(), random_levels.end(), 0);
    std::vector<uint16_t> random_levels_2(random_levels);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(random_levels.begin(), random_levels.end(), g);
    std::shuffle(random_levels_2.begin(), random_levels_2.end(), g);

    common::decimal_price decimal_price;
    common::decimal_quantity decimal_quantity;
    for (auto _ : state) {
        for (uint16_t level : random_levels) {
            decimal_price.set_underlying(level);
            decimal_quantity.set_underlying(level);
            tracker.report_quantity(
                common::Side::sell, decimal_quantity, decimal_price
            );
        }
        for (uint16_t level : random_levels_2) {
            decimal_price.set_underlying(level);
            benchmark::DoNotOptimize(
                tracker.get_level(common::Side::sell, decimal_price)
            );
        }
    }
}

BENCHMARK(BM_RandomIterate)->Range(128, 128 << 10);

BENCHMARK_MAIN();
