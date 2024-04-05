#include "retail.hpp"

#include "exchange/config.h"

namespace nutc {
namespace bots {

bool
RetailBot::is_active() const
{
    return get_capital()
           > -get_interest_limit() * .9; // drop out if they lose 90% of their money
}

std::optional<messages::MarketOrder>
RetailBot::take_action(double current, double theo)
{
    if (!is_active()) {
        return std::nullopt;
    }
    double p_trade = (1 - get_capital_utilization());

    std::uniform_real_distribution<> dis(0.0, 1.0);
    double noise_factor = dis(gen);

    double signal_strength = AGGRESSIVENESS * std::abs(theo - current) / current;

    if (poisson_dist(gen) > 0) {
        if (noise_factor < p_trade * signal_strength) {
            if (current < theo) {
                double price = current + RETAIL_ORDER_OFFSET;
                assert(price > 0);
                double quantity =
                    (1 - get_capital_utilization()) * get_interest_limit() / price;
                quantity *= RETAIL_ORDER_SIZE;
                modify_open_bids(1);
                modify_long_capital(quantity * price);
                return messages::MarketOrder{
                    get_id(), messages::SIDE::BUY, TICKER, quantity, price
                };
            }
            if (current > theo) {
                double price = current - RETAIL_ORDER_OFFSET;
                assert(price > 0);
                double quantity =
                    (1 - get_capital_utilization()) * get_interest_limit() / price;
                quantity *= RETAIL_ORDER_SIZE;
                modify_open_asks(1);
                modify_short_capital(quantity * price);
                return messages::MarketOrder{
                    get_id(), messages::SIDE::SELL, TICKER, quantity, price
                };
            }
        }
    }
    // hold
    return std::nullopt;
}

} // namespace bots
} // namespace nutc
