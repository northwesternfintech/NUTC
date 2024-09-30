#pragma once

#include "common/messages_wrapper_to_exchange.hpp"
#include "common/types/decimal.hpp"
#include "common/types/position.hpp"

#include <absl/hash/hash.h>
#include <boost/process.hpp>

#include <string>

namespace nutc::exchange {

class GenericTrader {
    std::string user_id_;
    common::decimal_price initial_capital_;
    common::decimal_price capital_delta_;
    std::array<common::decimal_quantity, common::TICKERS.size()> holdings_{};

public:
    explicit GenericTrader(std::string user_id, common::decimal_price capital) :
        user_id_(std::move(user_id)), initial_capital_(capital)
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

    // For metrics purposes
    virtual const std::string& get_type() const = 0;

    virtual common::decimal_price
    get_capital() const
    {
        return initial_capital_ + capital_delta_;
    }

    // TODO: improve with find
    common::decimal_quantity
    get_holdings(common::Ticker ticker) const
    {
        auto ticker_index = std::to_underlying(ticker);
        assert(ticker_index < holdings_.size());
        return holdings_[ticker_index];
    }

    common::decimal_quantity
    modify_holdings(common::Ticker ticker, common::decimal_quantity change_in_holdings)
    {
        auto ticker_index = std::to_underlying(ticker);
        assert(ticker_index < holdings_.size());
        return holdings_[ticker_index] += change_in_holdings;
    }

    void
    modify_capital(common::decimal_price change_in_capital)
    {
        capital_delta_ += change_in_capital;
    }

    common::decimal_price
    get_capital_delta() const
    {
        return capital_delta_;
    }

    common::decimal_price
    get_initial_capital() const
    {
        return initial_capital_;
    }

    virtual void notify_position_change(common::position) = 0;
    virtual void notify_match(common::position);
    virtual void send_message(const std::string&) = 0;

    using IncomingMessageQueue = std::vector<common::IncomingMessageVariant>;
    virtual IncomingMessageQueue read_orders() = 0;
};
} // namespace nutc::exchange
