#include "market_maker.hpp"

#include <algorithm>

namespace nutc {
namespace bots {
void
MarketMakerBot::process_order_match(
    const std::string& ticker, messages::SIDE side, double price, double quantity
)
{
    BotTrader::process_order_match(ticker, side, price, quantity);
}

void
MarketMakerBot::process_order_expiration(
    const std::string& ticker, messages::SIDE side, double price, double quantity
)
{
    BotTrader::process_order_expiration(ticker, side, price, quantity);
}

double
MarketMakerBot::avg_level_price(double new_theo)
{
    double total_price = 0;
    for (const price_level& level : BID_LEVELS) {
        total_price += (new_theo + level.PRICE_DELTA) * level.QUANTITY_FACTOR;
    }
    for (const price_level& level : ASK_LEVELS) {
        total_price += (new_theo + level.PRICE_DELTA) * level.QUANTITY_FACTOR;
    }

    return total_price;
}

std::vector<matching::StoredOrder>
MarketMakerBot::take_action(double new_theo, uint64_t current_tick)
{
    double capital_tolerance = compute_capital_tolerance_();
    double average_price = avg_level_price(new_theo);
    double total_quantity = capital_tolerance / average_price;

    auto trader =
        std::static_pointer_cast<manager::GenericTrader>(this->shared_from_this());

    std::vector<matching::StoredOrder> orders;
    orders.reserve(LEVELS * 2);

    auto create_order = [&](const price_level& level, const messages::SIDE side) {
        double price = new_theo + level.PRICE_DELTA;
        double quantity = total_quantity * level.QUANTITY_FACTOR;
        return matching::StoredOrder{trader,   side,  this->TICKER,
                                     quantity, price, current_tick};
    };
    std::ranges::transform(BID_LEVELS, std::back_inserter(orders), [&](auto level) {
        return create_order(level, messages::SIDE::BUY);
    });
    std::ranges::transform(ASK_LEVELS, std::back_inserter(orders), [&](auto level) {
        return create_order(level, messages::SIDE::SELL);
    });

    for (const auto& order : orders) {
        if (order.side == messages::SIDE::BUY) {
            modify_open_bids(order.quantity);
            modify_long_capital(order.quantity * order.price);
        }
        else {
            modify_open_asks(order.quantity);
            modify_short_capital(order.quantity * order.price);
        }
    }

    return orders;
}
} // namespace bots
} // namespace nutc
