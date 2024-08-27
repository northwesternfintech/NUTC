#pragma once

#include "common/messages_wrapper_to_exchange.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"

#include <fmt/format.h>

namespace nutc::test {

// Basically a generic trader but
class TestTrader final : public exchange::GenericTrader {
    IncomingMessageQueue pending_orders_;

public:
    TestTrader(double capital) : TestTrader("TEST", capital) {}

    TestTrader(std::string trader_id, double capital) :
        GenericTrader(std::move(trader_id), capital)
    {}

    void
    disable() final
    {}

    void
    send_message(const std::string&) final
    {}

    virtual void
    notify_position_change(common::position) final
    {}

    IncomingMessageQueue
    read_orders() final
    {
        IncomingMessageQueue ret{};
        pending_orders_.swap(ret);
        return ret;
    }

    void
    add_order(common::limit_order order)
    {
        pending_orders_.push_back(common::timed_limit_order{order});
    }

    void
    add_order(common::market_order order)
    {
        pending_orders_.push_back(common::timed_market_order{order});
    }

    const std::string&
    get_type() const final
    {
        static const std::string TYPE = "TEST";
        return TYPE;
    }
};

} // namespace nutc::test
