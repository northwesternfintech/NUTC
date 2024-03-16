#pragma once

#include "trader_type.hpp"

#include <string>
#include <unordered_map>

namespace nutc {
namespace manager {

class GenericTrader {
public:
    std::string
    get_id() const
    {
        return USER_ID;
    }

    virtual TraderType get_type() const = 0;

    double
    get_capital() const
    {
        return capital_;
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

    double
    modify_holdings(const std::string& ticker, double change_in_holdings)
    {
        holdings_[ticker] += change_in_holdings;
        return holdings_[ticker];
    }

    void
    set_capital(double capital)
    {
        capital_ = capital;
    }

    void
    modify_capital(double change_in_capital)
    {
        capital_ += change_in_capital;
    }

    double
    get_pnl() const
    {
        return capital_ - INITIAL_CAPITAL;
    }

    virtual void set_pid(pid_t pid) = 0;
    virtual pid_t get_pid() const = 0;

    virtual std::string get_algo_id() const = 0;

    explicit GenericTrader(std::string user_id, double capital) :
        USER_ID(std::move(user_id)), INITIAL_CAPITAL(capital), capital_(capital)
    {}

    virtual ~GenericTrader() = default;
    GenericTrader& operator=(GenericTrader&& other) = delete;
    GenericTrader& operator=(const GenericTrader& other) = delete;
    GenericTrader(GenericTrader&& other) = default;
    GenericTrader(const GenericTrader& other) = delete;

private:
    const std::string USER_ID;
    const double INITIAL_CAPITAL;
    double capital_;
    bool is_active_ = false;
    bool has_start_delay_ = true;
    std::unordered_map<std::string, double> holdings_{};
};
} // namespace manager
} // namespace nutc
