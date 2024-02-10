#pragma once
#include "generic_bot.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <sys/types.h>

namespace nutc {
namespace bots {

class MarketMakerBot : public GenericBot {
public:
    MarketMakerBot(std::string bot_id, float interest_limit) :
        GenericBot(std::move(bot_id), interest_limit)
    {}

    static constexpr float BASE_SPREAD = 0.16f;

    std::vector<messages::MarketOrder>
    take_action(float new_theo)
    {
        float long_interest = get_long_interest();
        float short_interest = get_short_interest();

        float interest_limit = get_interest_limit();

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
            -1 * ((long_interest - short_interest) / (long_interest + short_interest))
            * interest_limit * 2.7; // NOLINT(*)

        if (true || long_interest + short_interest == 0)
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
            auto side = (i < LEVELS / 2) ? messages::SIDE::BUY : messages::SIDE::SELL;
            orders[i] = messages::MarketOrder{
                GenericBot::get_id(), side, "", total_quantity * quantities[i],
                prices[i]
            };
            if (side == messages::SIDE::BUY) {
                modify_long_capital(total_quantity * quantities[i] * prices[i]);
            }
            else {
                modify_short_capital(total_quantity * quantities[i] * prices[i]);
            }
        }
        modify_open_bids(LEVELS / 2);
        modify_open_asks(LEVELS / 2);

        return orders;
    }

    [[nodiscard]] float
    get_utilization() const override
    {
        return (get_long_interest() + get_short_interest()) / get_interest_limit();
    }

private:
    [[nodiscard]] float
    compute_net_exposure_() const
    {
        return (get_long_interest() - get_short_interest());
    }

    float
    compute_capital_tolerance_()
    {
        return (1 - get_utilization()) * (get_interest_limit() / 3);
    }
};

} // namespace bots
} // namespace nutc
