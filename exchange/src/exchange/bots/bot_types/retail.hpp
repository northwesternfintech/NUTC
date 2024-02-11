#pragma once

#include "generic_bot.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

namespace nutc {

namespace bots {

class RetailBot : public GenericBot {
    std::random_device rd;
    std::mt19937 gen;
    std::poisson_distribution<> poisson_dist;

public:
    RetailBot(std::string bot_id, float interest_limit) :
        GenericBot(std::move(bot_id), interest_limit)
    {}

    std::optional<messages::MarketOrder>
    take_action(float current, float theo)
    {
        float p_trade = (1 - get_capital_utilization());

        std::uniform_real_distribution<> dis(0.0, 1.0);
        float noise_factor = dis(gen);

        float signal_strength = std::abs(theo - current) / current;

        if (poisson_dist(gen) > 0) {
            if (noise_factor < p_trade * signal_strength) {
                if (current < theo) {
                    float price = calculate_order_price(messages::SIDE::BUY, current, theo);
                    float quantity =
                        get_capital_utilization() * .667f * get_interest_limit() / price;
                    return messages::MarketOrder{
                        get_id(), messages::SIDE::BUY, "", quantity, price
                    };
                    // buy
                }
                if (current > theo) {
                    // sell
                    float price = calculate_order_price(messages::SIDE::SELL, current, theo);
                    float quantity =
                        get_capital_utilization() * .667f * get_interest_limit() / price;
                    return messages::MarketOrder{
                        get_id(), messages::SIDE::SELL, "", quantity, price
                    };
                }
            }
        }
        // hold
        return std::nullopt;
    }

private:
    static float
    calculate_order_price(
        messages::SIDE side, float current_price, float theo_price,
        float buffer_percent = 0.02f
    )
    {
        float price_difference = theo_price - current_price;

        float signal_strength_adjustment = price_difference * 0.1f;

        float buffer_amount = current_price * buffer_percent;

        if (side == messages::SIDE::BUY) {
            return current_price - buffer_amount - signal_strength_adjustment;
        }
        return current_price + buffer_amount + signal_strength_adjustment;
    }
};

} // namespace bots

} // namespace nutc
