#pragma once

#include "exchange/traders/trader_types/generic_trader.hpp"

#include <fmt/format.h>

namespace nutc {
namespace test_utils {

// Basically a generic trader but
class TestTrader final : public traders::GenericTrader {
    std::vector<messages::limit_order> pending_orders_;

public:
    TestTrader(double capital) : TestTrader("TEST", capital) {}

    TestTrader(std::string trader_id, double capital) :
        GenericTrader(std::move(trader_id), capital)
    {}

    void
    send_message(const std::string&) override
    {}

    virtual void
    process_position_change(util::position) override
    {}

    std::vector<messages::limit_order>
    read_orders() override
    {
        auto ret = std::move(pending_orders_);
        pending_orders_.clear();
        return ret;
    }

    void
    add_order(messages::limit_order order)
    {
        pending_orders_.push_back(order);
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
