#pragma once

#include <string>

namespace nutc {
namespace bots {

class GenericBot {
    const std::string BOT_ID;
    float short_interest_ = 0;
    float long_interest_ = 0;

    int open_bids_ = 0; // for stats, not the strategy
    int open_asks_ = 0;

    float interest_limit_;

public:
    [[nodiscard]] const std::string&
    get_id() const
    {
        return BOT_ID;
    }

    void
    modify_short_capital(float delta)
    {
        short_interest_ += delta;
    }

    void
    modify_long_capital(float delta)
    {
        long_interest_ += delta;
    }

    [[nodiscard]] float
    get_long_interest() const
    {
        return long_interest_;
    }

    [[nodiscard]] float
    get_interest_limit() const
    {
        return interest_limit_;
    }

    [[nodiscard]] float
    get_short_interest() const
    {
        return short_interest_;
    }

    [[nodiscard]] int
    get_open_bids() const
    {
        return open_bids_;
    }

    [[nodiscard]] int
    get_open_asks() const
    {
        return open_asks_;
    }

    void
    modify_open_bids(int delta)
    {
        open_bids_ += delta;
    }

    void
    modify_open_asks(int delta)
    {
        open_asks_ += delta;
    }

    [[nodiscard]] virtual float get_utilization() const = 0;

    virtual ~GenericBot() = default;

    GenericBot(std::string bot_id, float interest_limit) :
        BOT_ID(std::move(bot_id)), interest_limit_(interest_limit)
    {}

    GenericBot(const GenericBot& other) = default;
    GenericBot(GenericBot&& other) = default;
    GenericBot& operator=(const GenericBot& other) = delete;
    GenericBot& operator=(GenericBot&& other) = delete;
};

} // namespace bots
} // namespace nutc
