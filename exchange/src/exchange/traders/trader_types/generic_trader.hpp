#pragma once

#include "hash_table7.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "shared/types/decimal_price.hpp"
#include "shared/types/position.hpp"

#include <absl/hash/hash.h>
#include <boost/process.hpp>

#include <queue>
#include <string>

namespace nutc {
namespace traders {

class GenericTrader {
    const std::string USER_ID;
    const util::decimal_price INITIAL_CAPITAL;
    util::decimal_price capital_delta_{};
    emhash7::HashMap<util::Ticker, double, absl::Hash<util::Ticker>> holdings_{};

public:
    explicit GenericTrader(std::string user_id, util::decimal_price capital) :
        USER_ID(std::move(user_id)), INITIAL_CAPITAL(capital)
    {}

    virtual void
    disable()
    {}

    virtual ~GenericTrader() = default;

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

    virtual util::decimal_price
    get_capital() const
    {
        return INITIAL_CAPITAL + capital_delta_;
    }

    double
    get_holdings(util::Ticker ticker) const
    {
        if (!holdings_.contains(ticker))
            return 0.0;

        return holdings_.at(ticker);
    }

    double
    modify_holdings(util::Ticker ticker, double change_in_holdings)
    {
        holdings_[ticker] += change_in_holdings;
        return holdings_[ticker];
    }

    void
    modify_capital(util::decimal_price change_in_capital)
    {
        capital_delta_ += change_in_capital;
    }

    util::decimal_price
    get_capital_delta() const
    {
        return capital_delta_;
    }

    util::decimal_price
    get_initial_capital() const
    {
        return INITIAL_CAPITAL;
    }

    virtual void process_position_change(util::position) = 0;
    virtual void process_order_match(util::position);

    virtual void send_message(const std::string&) = 0;

    using MessageQueue = std::vector<std::variant<
        messages::timed_init_message, messages::timed_limit_order,
        messages::timed_market_order>>;

    virtual MessageQueue read_orders() = 0;
};
} // namespace traders
} // namespace nutc
