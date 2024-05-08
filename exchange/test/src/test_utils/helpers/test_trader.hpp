#pragma once

#include "exchange/traders/trader_types/trader_interface.hpp"

#include <fmt/format.h>

namespace nutc {
namespace test_utils {

// Basically a generic trader but
class TestTrader : public traders::GenericTrader {
public:
    TestTrader(const std::string& trader_id, double capital) :
        GenericTrader(trader_id, capital)
    {}

    void
    send_messages(const std::vector<std::string>&) override
    {}

    std::vector<market_order>
    read_orders() override
    {
        return {};
    }
};

} // namespace test_utils
} // namespace nutc
