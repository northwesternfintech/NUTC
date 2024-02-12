#pragma once
#include "generic_bot.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <sys/types.h>

namespace nutc {
namespace bots {

class MarketMakerBot : public GenericBot {
public:
    MarketMakerBot(std::string bot_id, double interest_limit) :
        GenericBot(std::move(bot_id), interest_limit)
    {}

    static constexpr double BASE_SPREAD = 0.16;

    std::vector<messages::MarketOrder>
    take_action(double new_theo)
    {
        double long_interest = get_long_interest();
        double short_interest = get_short_interest();

        double interest_limit = get_interest_limit();

        static constexpr uint8_t LEVELS = 6;
        std::vector<messages::MarketOrder> orders(LEVELS);

        std::array<double, LEVELS> quantities = {1.0 / 12, 1.0 / 6, 1.0 / 4,
                                                 1.0 / 4,  1.0 / 6, 1.0 / 12};

        std::array<double, LEVELS> prices = {
            new_theo - BASE_SPREAD - .10, new_theo - BASE_SPREAD - .05,
            new_theo - BASE_SPREAD,        new_theo + BASE_SPREAD,
            new_theo + BASE_SPREAD + .05, new_theo + BASE_SPREAD * .10,
        };

        double capital_tolerance = compute_capital_tolerance_();
        double lean =
            -1 * ((long_interest - short_interest) / (long_interest + short_interest))
            * interest_limit * 2.7; // NOLINT(*)

        if (true || long_interest + short_interest == 0)
            lean = 0;
        for (auto& price : prices) {
            price += lean;
        }

        double avg_price = 0;
        for (size_t i = 0; i < LEVELS; ++i) {
            avg_price += prices[i] * quantities[i];
        }

        double total_quantity = capital_tolerance / avg_price;

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

private:
    [[nodiscard]] double
    compute_net_exposure_() const
    {
        return (get_long_interest() - get_short_interest());
    }

    double
    compute_capital_tolerance_()
    {
        return (1 - get_capital_utilization()) * (get_interest_limit() / 3);
    }
};

} // namespace bots
} // namespace nutc
