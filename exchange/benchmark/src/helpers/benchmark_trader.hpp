#pragma once

#include "exchange/traders/trader_types/generic_trader.hpp"

#include <benchmark/benchmark.h>
#include <fmt/format.h>

namespace nutc {
namespace bench_utils {

class BenchmarkTrader : public traders::GenericTrader {
public:
    BenchmarkTrader() : BenchmarkTrader("Benchmark", 0.0) {}

    BenchmarkTrader(std::string trader_id, double capital) :
        GenericTrader(std::move(trader_id), capital)
    {}

    void
    send_message(const std::string&) override
    {}

    virtual void
    process_position_change(util::position change) override
    {
        benchmark::DoNotOptimize(change);
    }

    std::vector<messages::limit_order>
    read_orders() override
    {
        return {};
    }

    void
    add_order(messages::limit_order order)
    {
        benchmark::DoNotOptimize(order);
    }

    const std::string&
    get_type() const final
    {
        static const std::string TYPE = "TEST";
        return TYPE;
    }
};

} // namespace bench_utils
} // namespace nutc
