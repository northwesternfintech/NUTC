#pragma once

#include "shared/messages_wrapper_to_exchange.hpp"

#include <boost/process.hpp>

#include <string>
#include <unordered_map>

namespace nutc {
namespace traders {

using market_order = messages::market_order;

class GenericTrader {
    const std::string USER_ID;
    const double INITIAL_CAPITAL;
    double capital_delta_{};
    std::unordered_map<std::string, double> holdings_{};

public:
    explicit GenericTrader(std::string user_id, double capital) :
        USER_ID(std::move(user_id)), INITIAL_CAPITAL(capital)
    {}

    virtual ~GenericTrader() = default;
    GenericTrader& operator=(GenericTrader&& other) = delete;
    GenericTrader& operator=(const GenericTrader& other) = delete;
    GenericTrader(GenericTrader&& other) = default;
    GenericTrader(const GenericTrader& other) = delete;

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
    get_holdings(const std::string& ticker) const
    {
        if (!holdings_.contains(ticker))
            return 0.0;

        return holdings_.at(ticker);
    }

    double
    modify_holdings(const std::string& ticker, double change_in_holdings)
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

    virtual void process_order_remove(market_order) = 0;
    virtual void process_order_add(market_order) = 0;
    virtual void process_order_match(market_order);

    virtual void send_message(const std::string&) = 0;

    virtual std::vector<market_order> read_orders() = 0;
};
} // namespace traders
} // namespace nutc