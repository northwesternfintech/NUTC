#pragma once

#include "hash_table7.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <absl/hash/hash.h>
#include <boost/process.hpp>

#include <string>

namespace nutc {
namespace traders {

class GenericTrader {
    const std::string USER_ID;
    const double INITIAL_CAPITAL;
    double capital_delta_{};
    emhash7::HashMap<util::Ticker, double, absl::Hash<util::Ticker>> holdings_{};

public:
    explicit GenericTrader(std::string user_id, double capital) :
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

    virtual double
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
    modify_capital(double change_in_capital)
    {
        capital_delta_ += change_in_capital;
    }

    double
    get_capital_delta() const
    {
        return capital_delta_;
    }

    double
    get_initial_capital() const
    {
        return INITIAL_CAPITAL;
    }

    virtual void process_position_change(util::position) = 0;
    virtual void process_order_match(util::position);

    virtual void send_message(const std::string&) = 0;

    virtual std::vector<messages::limit_order> read_orders() = 0;
};
} // namespace traders
} // namespace nutc
