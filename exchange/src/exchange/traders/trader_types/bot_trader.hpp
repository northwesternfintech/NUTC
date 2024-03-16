#pragma once

#include "exchange/traders/trader_types/generic_trader.hpp"

#include <cassert>

#include <stdexcept>
#include <string>

namespace nutc {
namespace bots {

class BotTrader : public manager::GenericTrader {
    static uint
    get_and_increment_user_id()
    {
        static uint user_id = 0;
        return user_id++;
    }

    static std::string
    generate_user_id()
    {
        return "BOT_" + std::to_string(get_and_increment_user_id());
    }

protected:
    const std::string TICKER;
    double short_interest_ = 0;
    double long_interest_ = 0;

    size_t open_bids_ = 0; // for stats, not the strategy
    size_t open_asks_ = 0;

    const double INTEREST_LIMIT;

public:
    constexpr manager::TraderType
    get_type() const override
    {
        return manager::TraderType::BOT;
    }

    pid_t
    get_pid() const override
    {
        return -1;
    }

    void
    set_pid(pid_t) override
    {}

    const std::string&
    get_algo_id() const override
    {
        throw std::runtime_error("Not implemented");
    }

    [[nodiscard]] double
    get_held_stock() const
    {
        return get_holdings(TICKER);
    }

    void
    modify_held_stock(double delta)
    {
        modify_holdings(TICKER, delta);
    }

    void
    modify_short_capital(double delta)
    {
        short_interest_ += delta;
    }

    void
    modify_long_capital(double delta)
    {
        long_interest_ += delta;
    }

    [[nodiscard]] double
    get_long_interest() const
    {
        return long_interest_;
    }

    [[nodiscard]] double
    get_interest_limit() const
    {
        return INTEREST_LIMIT;
    }

    [[nodiscard]] double
    get_short_interest() const
    {
        return short_interest_;
    }

    [[nodiscard]] size_t
    get_open_bids() const
    {
        return open_bids_;
    }

    [[nodiscard]] size_t
    get_open_asks() const
    {
        return open_asks_;
    }

    [[nodiscard]] double
    get_capital_utilization() const
    {
        return (get_long_interest() + get_short_interest()) / get_interest_limit();
    }

    void
    modify_open_bids(int delta)
    {
        if (delta < 0) {
            assert(open_bids_ >= static_cast<size_t>(std::abs(delta)));
            open_bids_ -= static_cast<size_t>(std::abs(delta));
        }
        else {
            open_bids_ += static_cast<size_t>(delta);
        }
    }

    void
    modify_open_asks(int delta)
    {
        if (delta < 0) {
            assert(open_asks_ >= static_cast<size_t>(std::abs(delta)));
            open_asks_ -= static_cast<size_t>(std::abs(delta));
        }
        else {
            open_asks_ += static_cast<size_t>(delta);
        }
    }

    ~BotTrader() override = default;

    [[nodiscard]] virtual bool
    is_active() const
    {
        return false;
    }

    BotTrader(std::string ticker, double interest_limit) :
        GenericTrader(generate_user_id(), interest_limit), TICKER(std::move(ticker)),
        INTEREST_LIMIT(interest_limit)
    {}

    BotTrader(const BotTrader& other) = delete;
    BotTrader(BotTrader&& other) = default;
    BotTrader& operator=(const BotTrader& other) = delete;
    BotTrader& operator=(BotTrader&& other) = delete;
};

} // namespace bots
} // namespace nutc
