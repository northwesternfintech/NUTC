#pragma once

#include "shared/util.hpp"

#include <cassert>

#include <string>
#include <unordered_map>

namespace nutc {
namespace manager {
enum TraderType { REMOTE, LOCAL, BOT };

class GenericTrader {
    const std::string USER_ID;
    const double INITIAL_CAPITAL;
    double capital_delta_ = 0;
    bool is_active_ = false;
    bool has_start_delay_ = true;
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

    const std::string&
    get_id() const
    {
        return USER_ID;
    }

    virtual TraderType get_type() const = 0;

    virtual double
    get_capital() const
    {
        return INITIAL_CAPITAL + capital_delta_;
    }

    bool
    is_active() const
    {
        return is_active_;
    }

    void
    set_active(bool active)
    {
        is_active_ = active;
    }

    bool
    has_start_delay() const
    {
        return has_start_delay_;
    }

    void
    set_start_delay(bool start_delay)
    {
        has_start_delay_ = start_delay;
    }

    double
    get_holdings(const std::string& ticker) const
    {
        if (!holdings_.contains(ticker))
            return 0.0;

        return holdings_.at(ticker);
    }

    virtual constexpr bool
    can_leverage() const
    {
        return false;
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
        // assert(can_leverage() || get_capital() >= -1);
    }

    double
    get_capital_delta() const
    {
        return capital_delta_;
    }

    // will be removed soon, making these changes iteratively
    virtual void
    process_order_add(const std::string&, messages::SIDE, double, double)
    {}

    // For now, only bots care about this
    // ticker, side, price, quantity
    virtual void
    process_order_expiration(const std::string&, messages::SIDE, double, double)
    {}

    // ticker, price, side, quantity
    virtual void process_order_match(
        const std::string& ticker, messages::SIDE side, double price, double quantity
    );

    virtual void set_pid(const pid_t& pid) = 0;
    virtual pid_t get_pid() const = 0;

    virtual const std::string& get_algo_id() const = 0;
};
} // namespace manager
} // namespace nutc
