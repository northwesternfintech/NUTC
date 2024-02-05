#pragma once
#include "exchange/logging.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <vector>

namespace nutc {
namespace bots {

class MarketMakerBot {
public:
    explicit MarketMakerBot(std::string bot_id, float interest_limit) :
        interest_limit_(interest_limit), bot_id_(std::move(bot_id))
    {}

    static constexpr float BASE_SPREAD = 0.16f;

    // Do something better later
    MarketMakerBot() = default;

    std::vector<messages::MarketOrder>
    take_action(float new_theo)
    {
        static constexpr uint8_t LEVELS = 6;
        std::vector<messages::MarketOrder> orders(LEVELS);

        std::array<float, LEVELS> quantities = {1.0f / 12, 1.0f / 6, 1.0f / 4,
                                                1.0f / 4,  1.0f / 6, 1.0f / 12};

        std::array<float, LEVELS> prices = {
            new_theo - BASE_SPREAD - .10f, new_theo - BASE_SPREAD - .05f,
            new_theo - BASE_SPREAD,        new_theo + BASE_SPREAD,
            new_theo + BASE_SPREAD + .05f, new_theo + BASE_SPREAD * .10f,
        };

        float capital_tolerance = compute_capital_tolerance_();
        float lean =
            -1
            * ((long_interest_ - short_interest_) / (long_interest_ + short_interest_))
            * interest_limit_ * 2.7; // NOLINT(*)

        if (true || long_interest_ + short_interest_ == 0)
            lean = 0;
        for (auto& price : prices) {
            price += lean;
        }

        float avg_price = 0;
        for (size_t i = 0; i < LEVELS; ++i) {
            avg_price += prices[i] * quantities[i];
        }

        float total_quantity = capital_tolerance / avg_price;

        for (size_t i = 0; i < LEVELS; ++i) {
            messages::SIDE side =
                (i < LEVELS / 2) ? messages::SIDE::BUY : messages::SIDE::SELL;
            orders[i] = messages::MarketOrder{
                bot_id_, side, "", total_quantity * quantities[i], prices[i]
            };
            if (side == messages::SIDE::BUY) {
                modify_long_capital(total_quantity * quantities[i] * prices[i]);
            }
            else {
                modify_short_capital(total_quantity * quantities[i] * prices[i]);
            }
        }

        return orders;
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

private:
    float long_interest_ = 0;
    float short_interest_ = 0;

    float interest_limit_;

    std::string bot_id_;

    [[nodiscard]] float
    compute_net_exposure_() const
    {
        return (long_interest_ - short_interest_);
    }

    [[nodiscard]] float
    compute_capital_util_() const
    {
        return (long_interest_ + short_interest_) / interest_limit_;
    }

    float
    compute_capital_tolerance_()
    {
        return (1 - compute_capital_util_()) * (interest_limit_ / 3);
    }
};

} // namespace bots
} // namespace nutc
