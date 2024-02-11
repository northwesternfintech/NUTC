#include "retail.hpp"

namespace nutc {
namespace bots {

std::optional<messages::MarketOrder>
RetailBot::take_action(double current, double theo)
{
    double p_trade = (1 - get_capital_utilization());

    std::uniform_real_distribution<> dis(0.0, 1.0);
    double noise_factor = dis(gen);

    double signal_strength = AGGRESSIVENESS * std::abs(theo - current) / current;

    if (poisson_dist(gen) > 0) {
        if (noise_factor < p_trade * signal_strength) {
            if (current < theo) {
                // double price = calculate_order_price(messages::SIDE::BUY, current,
                // theo);
                double price = current;
                double quantity = (1 - get_capital_utilization()) * .4f
                                  * get_interest_limit() / price;
                modify_open_bids(1);
                modify_long_capital(quantity * price);
                return messages::MarketOrder{
                    get_id(), messages::SIDE::BUY, "", quantity, price
                };
                // buy
            }
            if (current > theo) {
                // calculate_order_price(messages::SIDE::SELL, current, theo);
                double price = current;
                double quantity = (1 - get_capital_utilization()) * .4f
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

double
RetailBot::calculate_order_price(
    messages::SIDE side, double current_price, double theo_price, double buffer_percent
)
{
    double price_difference = std::abs(theo_price - current_price);

    double signal_strength_adjustment = price_difference * 0.5f;

    double buffer_amount = current_price * buffer_percent;

    if (side == messages::SIDE::BUY) {
        return current_price - buffer_amount - signal_strength_adjustment;
    }
    return current_price + buffer_amount + signal_strength_adjustment;
}

} // namespace bots
} // namespace nutc
