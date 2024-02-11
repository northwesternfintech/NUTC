#include "retail.hpp"

namespace nutc {
namespace bots {

std::optional<messages::MarketOrder>
RetailBot::take_action(float current, float theo)
{
    float p_trade = (1 - get_capital_utilization());

    std::uniform_real_distribution<> dis(0.0, 1.0);
    float noise_factor = dis(gen);

    float signal_strength = 1000 * std::abs(theo - current) / current;

    if (poisson_dist(gen) > 0) {
        if (true || noise_factor < p_trade * signal_strength) {
            if (current < theo) {
                // float price = calculate_order_price(messages::SIDE::BUY, current, theo);
                float price = current;
                assert(price > 0);
                float quantity = (1 - get_capital_utilization()) * .4f
                                 * get_interest_limit() / price;
                modify_open_bids(1);
                modify_long_capital(quantity * price);
                return messages::MarketOrder{
                    get_id(), messages::SIDE::BUY, "", quantity, price
                };
                // buy
            }
            if (current > theo) {
                // sell
                // float price =
                    // calculate_order_price(messages::SIDE::SELL, current, theo);
                float price = current;
                assert(price > 0);
                float quantity = (1 - get_capital_utilization()) * .4f
                                 * get_interest_limit() / price;
                modify_open_asks(1);
                modify_short_capital(quantity * price);
                return messages::MarketOrder{
                    get_id(), messages::SIDE::SELL, "", quantity, price
                };
            }
        }
    }
    // hold
    return std::nullopt;
}

float
RetailBot::calculate_order_price(
    messages::SIDE side, float current_price, float theo_price, float buffer_percent
)
{
    float price_difference = std::abs(theo_price - current_price);

    float signal_strength_adjustment = price_difference * 0.5f;

    float buffer_amount = current_price * buffer_percent;

    if (side == messages::SIDE::BUY) {
        return current_price - buffer_amount - signal_strength_adjustment;
    }
    return current_price + buffer_amount + signal_strength_adjustment;
}

} // namespace bots
} // namespace nutc
