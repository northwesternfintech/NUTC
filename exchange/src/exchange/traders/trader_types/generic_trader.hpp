#pragma once

#include "common/messages_wrapper_to_exchange.hpp"
#include "common/types/decimal.hpp"
#include "common/types/position.hpp"
#include "hash_table7.hpp"

#include <absl/hash/hash.h>
#include <boost/process.hpp>

#include <string>

namespace nutc::exchange {

class GenericTrader {
    const std::string USER_ID;
    const common::decimal_price INITIAL_CAPITAL;
    common::decimal_price capital_delta_{};
    emhash7::HashMap<
        common::Ticker, common::decimal_quantity, absl::Hash<common::Ticker>>
        holdings_{};

public:
    explicit GenericTrader(std::string user_id, common::decimal_price capital) :
        USER_ID(std::move(user_id)), INITIAL_CAPITAL(capital)
    {}

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
        return USER_ID;
    }

    const std::string&
    get_id() const
    {
        return USER_ID;
    }

    // For metrics purposes
    virtual const std::string& get_type() const = 0;

    virtual common::decimal_price
    get_capital() const
    {
        return INITIAL_CAPITAL + capital_delta_;
    }

    // TODO: improve with find
    common::decimal_quantity
    get_holdings(common::Ticker ticker) const
    {
        auto holdings_it = holdings_.find(ticker);
        if (holdings_it == holdings_.end()) [[unlikely]]
            return 0.0;

        return holdings_it->second;
    }

    common::decimal_quantity
    modify_holdings(common::Ticker ticker, common::decimal_quantity change_in_holdings)
    {
        holdings_[ticker] += change_in_holdings;
        return holdings_[ticker];
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
        return INITIAL_CAPITAL;
    }

    virtual void notify_position_change(common::position) = 0;
    virtual void notify_match(common::position);
    virtual void send_message(const std::string&) = 0;

    using IncomingMessageQueue = std::vector<common::IncomingMessageVariant>;
    virtual IncomingMessageQueue read_orders() = 0;
};
} // namespace nutc::exchange
