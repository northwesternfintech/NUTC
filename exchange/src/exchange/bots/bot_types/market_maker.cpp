#include "market_maker.hpp"

#include <algorithm>
#include <array>

namespace {
struct price_level {
    const double price_delta;
    const double quantity_factor;

    consteval price_level(double price_delta, double quantity_factor) :
        price_delta(price_delta), quantity_factor(quantity_factor)
    {}
};

// TODO: parameterize
constexpr double BASE_SPREAD = 0.16;
constexpr uint8_t LEVELS = 3;
constexpr std::array<price_level, LEVELS> BID_LEVELS{
    price_level{-BASE_SPREAD - .10, 1.0 / 12},
    price_level{-BASE_SPREAD - .05, 1.0 / 6 },
    price_level{-BASE_SPREAD - .00, 1.0 / 4 }
};
constexpr std::array<price_level, LEVELS> ASK_LEVELS{
    price_level{BASE_SPREAD + .00, 1.0 / 4 },
    price_level{BASE_SPREAD + .05, 1.0 / 6 },
    price_level{BASE_SPREAD + .10, 1.0 / 12}
};

} // namespace

namespace nutc {
namespace bots {
void
MarketMakerBot::process_order_match(
    const std::string& ticker, util::Side side, double price, double quantity
)
{
    BotTrader::process_order_match(ticker, side, price, quantity);
}

void
MarketMakerBot::process_order_expiration(
    const std::string& ticker, util::Side side, double price, double quantity
)
{
    BotTrader::process_order_expiration(ticker, side, price, quantity);
}

constexpr double
MarketMakerBot::avg_level_price(double new_theo)
{
    double total_price = 0;
    for (const price_level& level : BID_LEVELS) {
        total_price += (new_theo + level.price_delta) * level.quantity_factor;
    }
    for (const price_level& level : ASK_LEVELS) {
        total_price += (new_theo + level.price_delta) * level.quantity_factor;
    }

    return total_price;
}

std::shared_ptr<manager::GenericTrader>
MarketMakerBot::get_self_pointer_()
{
    return std::static_pointer_cast<manager::GenericTrader>(shared_from_this());
}

std::vector<matching::StoredOrder>
MarketMakerBot::take_action(double new_theo, uint64_t current_tick)
{
    double capital_tolerance = compute_capital_tolerance_();
    double average_price = avg_level_price(new_theo);
    double total_quantity = capital_tolerance / average_price;

    auto trader = get_self_pointer_();

    std::vector<matching::StoredOrder> orders;
    orders.reserve(static_cast<size_t>(LEVELS) * 2);

    auto create_order = [&](const price_level& level, const util::Side side) {
        double price = new_theo + level.price_delta;
        double quantity = total_quantity * level.quantity_factor;
        return matching::StoredOrder{trader,   side,  this->TICKER,
                                     quantity, price, current_tick};
    };

    std::ranges::transform(BID_LEVELS, std::back_inserter(orders), [&](auto level) {
        return create_order(level, util::Side::buy);
    });
    std::ranges::transform(ASK_LEVELS, std::back_inserter(orders), [&](auto level) {
        return create_order(level, util::Side::sell);
    });

    auto update_position = [&](const matching::StoredOrder& order) {
        if (order.side == util::Side::buy) {
            modify_open_bids(order.quantity);
            modify_long_capital(order.quantity * order.price);
        }
        else {
            modify_open_asks(order.quantity);
            modify_short_capital(order.quantity * order.price);
        }
    };

    std::ranges::for_each(orders, update_position);

    return orders;
}
} // namespace bots
} // namespace nutc
