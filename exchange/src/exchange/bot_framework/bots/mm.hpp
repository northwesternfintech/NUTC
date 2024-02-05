#pragma once
#include "exchange/logging.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <vector>

namespace nutc {
namespace bots {

class MarketMakerBot {
public:
    explicit MarketMakerBot(std::string bot_id, float capital_limit) :
        capital_limit_(capital_limit), bot_id_(std::move(bot_id))
    {}

    static constexpr float BASE_SPREAD = 0.16f;

    // Do something better later
    MarketMakerBot() = default;

    std::vector<messages::MarketOrder>
    take_action(float new_theo, float current)
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
        float lean = compute_net_exposure_() / capital_tolerance / 8;
        for (auto& price : prices) {
            price += lean;
        }

        float avg_price = 0;
        for (size_t i = 0; i < LEVELS; ++i) {
            avg_price += prices[i] * quantities[i];
        }

        float total_quantity = LEVELS * capital_tolerance / avg_price;

        for (size_t i = 0; i < LEVELS; ++i) {
      messages::SIDE side = (i < LEVELS / 2) ? messages::SIDE::BUY : messages::SIDE::SELL;
            orders[i] = messages::MarketOrder{bot_id_, side, "", total_quantity * quantities[i], prices[i]};
        }

        return orders;

        /**
         * best bid and best ask is theo +/- base_spread/2
         * change price based on which side to move towards
         * do like 5 cent increments, if the levels arent getting hit
         *
         * in theory: cap tolerance is num money we'll spend on that tick
         * take net exposure, divide by cap limit, then divide by 8, thatll give how
         * much we should lean move all of the levels up that many cents base spread has
         * to be bigger than 16 cents
         *
         *
         */
    }

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

    std::string bot_id_;

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
        return (1 - compute_capital_util_()) * capital_limit_ / 3;
    }
};

} // namespace bots
} // namespace nutc
