#pragma once

#include "shared/util.hpp"

#include <boost/process.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace nutc {
namespace traders {

class GenericTrader : public std::enable_shared_from_this<GenericTrader> {
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

    /**
     * @brief Triggered when an order this bot created expires after
     * order_expiration_ticks, we want to let the trader know
     * @note For now, only bots care about this
     * @param ticker
     * @param quantity
     * @param price
     * @param quantity
     */
    virtual void
    process_order_expiration(const std::string&, util::Side, double, double)
    {}

    /**
     * @brief Triggered when an order this bot created matches
     * @note Implementing classes MUST update capital and holdings respectively
     */
    virtual void process_order_match(
        const std::string& ticker, util::Side side, double price, double quantity
    );

    virtual void
    send_messages(std::vector<std::string>)
    {}
};
} // namespace traders
} // namespace nutc
