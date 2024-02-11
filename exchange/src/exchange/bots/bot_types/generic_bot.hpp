#pragma once

#include <cassert>

#include <string>

namespace nutc {
namespace bots {

class GenericBot {
    const std::string BOT_ID;
    double short_interest_ = 0;
    double long_interest_ = 0;

    size_t open_bids_ = 0; // for stats, not the strategy
    size_t open_asks_ = 0;

    const double INTEREST_LIMIT;
    double capital_ = 0;
    double held_stock_ = 0;

public:
    [[nodiscard]] double
    get_capital() const
    {
        return capital_;
    }

    void
    modify_capital(double delta)
    {
        capital_ += delta;
    }

    [[nodiscard]] double
    get_held_stock() const
    {
        return held_stock_;
    }

    void
    modify_held_stock(double delta)
    {
        held_stock_ += delta;
    }

    [[nodiscard]] const std::string&
    get_id() const
    {
        return BOT_ID;
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

    virtual ~GenericBot() = default;

    GenericBot(std::string bot_id, double interest_limit) :
        BOT_ID(std::move(bot_id)), INTEREST_LIMIT(interest_limit)
    {}

    GenericBot(const GenericBot& other) = default;
    GenericBot(GenericBot&& other) = default;
    GenericBot& operator=(const GenericBot& other) = delete;
    GenericBot& operator=(GenericBot&& other) = delete;
};

} // namespace bots
} // namespace nutc
