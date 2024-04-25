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

std::optional<matching::stored_order>
RetailBot::take_action(double current, double theo, uint64_t current_tick)
{
    if (!is_active())
        return std::nullopt;

    static std::uniform_real_distribution<> dis{0.0, 1.0};

    double p_trade = (1 - get_capital_utilization());

    double noise_factor = dis(gen_);

    double signal_strength = AGGRESSIVENESS * std::abs(theo - current) / current;

    if (poisson_dist_(gen_) <= 0)
        return std::nullopt;
    if (noise_factor >= p_trade * signal_strength)
        return std::nullopt;

    auto trader =
        std::static_pointer_cast<traders::GenericTrader>(this->shared_from_this());

    if (current < theo) {
        double price = current + RETAIL_ORDER_OFFSET;
        assert(price > 0);
        double quantity =
            (1 - get_capital_utilization()) * get_interest_limit() / price;
        quantity *= RETAIL_ORDER_SIZE;
        modify_open_bids(quantity);
        modify_long_capital(quantity * price);
        return matching::stored_order{trader,   util::Side::buy, TICKER,
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
        return matching::stored_order{trader, util::Side::sell, TICKER, quantity,
                                      price,  current_tick};
    }
    return std::nullopt;
}

} // namespace bots
} // namespace nutc
