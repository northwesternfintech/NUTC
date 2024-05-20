#pragma once

#include "exchange/traders/trader_types/generic_trader.hpp"

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
    send_message(const std::string&) final
    {}

    virtual void
    process_order_remove(messages::market_order) final
    {}

    virtual void
    process_order_add(messages::market_order) final
    {}

    std::vector<messages::market_order>
    read_orders() override
    {
        return {};
    }

    const std::string&
    get_type() const final
    {
        static const std::string TYPE = "TEST";
        return TYPE;
    }
};

} // namespace test_utils
} // namespace nutc
