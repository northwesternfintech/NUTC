#include "retail.hpp"

#include "exchange/config.h"
#include "exchange/tickers/engine/order_storage.hpp"

#include <cstdint>

#include <memory>

namespace nutc {
namespace bots {

bool
RetailBot::is_active() const
{
    return get_capital()
           > -get_interest_limit() * .9; // drop out if they lose 90% of their money
}

std::optional<matching::StoredOrder>
RetailBot::take_action(double current, double theo, uint64_t current_tick)
{
    static std::uniform_real_distribution<> dis{0.0, 1.0};

    if (!is_active()) {
        return std::nullopt;
    }
    double p_trade = (1 - get_capital_utilization());

    double noise_factor = dis(gen_);

    double signal_strength = AGGRESSIVENESS * std::abs(theo - current) / current;

    if (poisson_dist_(gen_) > 0) {
        if (noise_factor < p_trade * signal_strength) {
            auto trader = std::static_pointer_cast<manager::GenericTrader>(
                this->shared_from_this()
            );
            if (current < theo) {
                double price = current + RETAIL_ORDER_OFFSET;
                assert(price > 0);
                double quantity =
                    (1 - get_capital_utilization()) * get_interest_limit() / price;
                quantity *= RETAIL_ORDER_SIZE;
                modify_open_bids(quantity);
                modify_long_capital(quantity * price);
                return matching::StoredOrder{trader,   util::Side::buy, TICKER,
                                             quantity, price,           current_tick};
            }
            if (current > theo) {
                double price = current - RETAIL_ORDER_OFFSET;
                assert(price > 0);
                double quantity =
                    (1 - get_capital_utilization()) * get_interest_limit() / price;
                quantity *= RETAIL_ORDER_SIZE;
                modify_open_asks(quantity);
                modify_short_capital(quantity * price);
                return matching::StoredOrder{trader, util::Side::sell, TICKER, quantity,
                                             price,  current_tick};
            }
        }
    }
    // hold
    return std::nullopt;
}

} // namespace bots
} // namespace nutc
