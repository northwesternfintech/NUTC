#pragma once

#include "exchange/traders/trader_types/generic_trader.hpp"
#include "shared/types/decimal.hpp"

#include <benchmark/benchmark.h>
#include <fmt/format.h>

namespace nutc {
namespace benchmarks {

class BenchmarkTrader : public exchange::GenericTrader {
public:
    BenchmarkTrader() : BenchmarkTrader("Benchmark", 0.0) {}

    BenchmarkTrader(std::string trader_id, shared::decimal_price capital) :
        GenericTrader(std::move(trader_id), capital)
    {}

    void
    send_message(const std::string& str) final
    {
        benchmark::DoNotOptimize(str.size());
    }

    void
    notify_position_change(shared::position change) final
    {
        benchmark::DoNotOptimize(change);
    }

    IncomingMessageQueue
    read_orders() final
    {
        return {};
    }

    void
    disable() final
    {}

    const std::string&
    get_type() const final
    {
        static const std::string type = "TEST";
        return type;
    }
};

} // namespace benchmarks
} // namespace nutc
