#pragma once

#include "common/messages_wrapper_to_exchange.hpp"
#include "common/types/decimal.hpp"
#include "common/types/position.hpp"
#include "exchange/traders/portfolio/trader_portfolio.hpp"

#include <absl/hash/hash.h>
#include <boost/process.hpp>

#include <string>

namespace nutc::exchange {

class GenericTrader {
    std::string user_id_;
    TraderPortfolio state_;

public:
    explicit GenericTrader(std::string user_id, common::decimal_price initial_capital) :
        user_id_(std::move(user_id)), state_{initial_capital}
    {}

    GenericTrader(GenericTrader&&) = default;
    GenericTrader(const GenericTrader&) = default;
    GenericTrader& operator=(GenericTrader&&) = default;
    GenericTrader& operator=(const GenericTrader&) = default;

    virtual ~GenericTrader() = default;

    virtual void disable() = 0;

    virtual bool
    can_leverage() const
    {
        return false;
    }

    virtual const std::string&
    get_display_name() const
    {
        return user_id_;
    }

    const std::string&
    get_id() const
    {
        return user_id_;
    }

    const TraderPortfolio&
    get_portfolio() const
    {
        return state_;
    }

    TraderPortfolio&
    get_portfolio()
    {
        return state_;
    }

    // For metrics purposes
    virtual const std::string& get_type() const = 0;

    virtual void send_message(const std::string&) = 0;

    using IncomingMessageQueue = std::vector<common::IncomingMessageVariant>;
    virtual IncomingMessageQueue read_orders() = 0;
};
} // namespace nutc::exchange
