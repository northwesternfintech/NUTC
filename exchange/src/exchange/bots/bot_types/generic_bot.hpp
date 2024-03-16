#pragma once

#include "exchange/traders/trader_types.hpp"

#include <cassert>

#include <string>

namespace nutc {
namespace bots {

class GenericBot : public manager::BotTrader {
protected:
    const std::string TICKER;
    double short_interest_ = 0;
    double long_interest_ = 0;

    size_t open_bids_ = 0; // for stats, not the strategy
    size_t open_asks_ = 0;

    const double INTEREST_LIMIT;

public:
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

    ~GenericBot() override = default;

    [[nodiscard]] virtual bool
    is_active() const
    {
        return false;
    }

    GenericBot(std::string ticker, double interest_limit) :
       BotTrader(interest_limit), TICKER(std::move(ticker)), INTEREST_LIMIT(interest_limit)
    {}

    GenericBot(const GenericBot& other) = delete;
    GenericBot(GenericBot&& other) = default;
    GenericBot& operator=(const GenericBot& other) = delete;
    GenericBot& operator=(GenericBot&& other) = delete;
};

} // namespace bots
} // namespace nutc
