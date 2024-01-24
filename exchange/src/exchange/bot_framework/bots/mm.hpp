#pragma once
#include "shared/messages_wrapper_to_exchange.hpp"

#include <vector>

namespace nutc {
namespace bots {

class MarketMakerBot {
public:
    explicit MarketMakerBot(float capital_limit) : capital_limit_(capital_limit) {}

    // Do something better later
    MarketMakerBot() : capital_limit_(1000) {}

    std::vector<messages::MarketOrder> take_action(float new_theo);

    void
    modify_short_capital(float delta)
    {
        short_capital_ += delta;
        assert(short_capital_ >= 0);
    }

    void
    modify_long_capital(float delta)
    {
        long_capital_ += delta;
        assert(long_capital_ >= 0);
    }

private:
    float long_capital_ = 0;
    float short_capital_ = 0;

    float capital_limit_;

    [[nodiscard]] float
    compute_net_exposure_() const
    {
        return (long_capital_ - short_capital_);
    }

    [[nodiscard]] float
    compute_capital_util_() const
    {
        return (long_capital_ + short_capital_) / capital_limit_;
    }

    float
    compute_capital_tolerance_()
    {
        return (-1 * compute_net_exposure_())
               * (1 + (1 - compute_capital_util_()) * capital_limit_ / 2);
    }
};

} // namespace bots
} // namespace nutc
