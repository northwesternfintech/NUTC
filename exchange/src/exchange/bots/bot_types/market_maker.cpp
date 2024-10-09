#include "market_maker.hpp"

#include "common/types/decimal.hpp"
#include "common/util.hpp"
#include "exchange/bots/shared_bot_state.hpp"

#include <algorithm>
#include <array>

using nutc::common::Side;

namespace {
struct price_level {
    const nutc::common::decimal_price PRICE_DELTA;
    const nutc::common::decimal_quantity QUANTITY_FACTOR;

    consteval price_level(
        nutc::common::decimal_price price_delta, double quantity_factor
    ) : PRICE_DELTA(price_delta), QUANTITY_FACTOR(quantity_factor)
    {}
};

// TODO: parameterize
constexpr double BASE_SPREAD = 0.16;
constexpr uint8_t LEVELS = 4;
constexpr std::array<price_level, LEVELS> PRICE_LEVELS{
    price_level{BASE_SPREAD + .00, 1.0 / 2 },
    price_level{BASE_SPREAD + .05, 1.0 / 3 },
    price_level{BASE_SPREAD + .10, 1.0 / 8 },
    price_level{BASE_SPREAD + .15, 1.0 / 24}
};

} // namespace

namespace nutc::exchange {

void
MarketMakerBot::place_orders_(
    Side side, decimal_price theo, decimal_price spread_offset
)
{
    // Approximation
    common::decimal_quantity total_quantity = {
        compute_capital_tolerance_() / (theo + spread_offset)
    };

    // Placing orders on both sides
    total_quantity /= 2.0;

    for (const auto& [price_delta, quantity_factor] : PRICE_LEVELS) {
        decimal_price price = (side == Side::buy) ? theo - price_delta - spread_offset
                                                  : theo + price_delta + spread_offset;

        if (price <= 0.0) [[unlikely]]
            return;

        auto order_id = add_limit_order(
            side, total_quantity * quantity_factor, price, /*ioc=*/false
        );
        open_orders.push_back(order_id);
    }
}

decimal_price
MarketMakerBot::calculate_lean_percent(const shared_bot_state& state)
{
    double ratio = double{state.CUMULATIVE_QUANTITY_HELD}
                   / double{state.CUMULATIVE_INTEREST_LIMIT};
    return ratio * static_cast<double>(state.MIDPRICE);
}

// TODO: clean up
void
MarketMakerBot::take_action(const shared_bot_state& state)
{
    std::ranges::for_each(open_orders, [this](auto order_id) {
        cancel_order(order_id);
    });
    open_orders.clear();
    // decimal_price spread_offset = state.MIDPRICE * (1.0 / 600 +
    // state.REALIZED_VOLATILITY); spread_offset *= aggressiveness; spread_offset =
    // std::min(spread_offset, 1.0); spread_offset = std::max(spread_offset, -1.0);
    //
    // TODO: adding market impact to the spread is very challenging and will be
    // continued later
    decimal_price spread_offset = 0.0;

    decimal_price lean = calculate_lean_percent(state);

    decimal_price theo = state.THEO - (lean * 1.0) + generate_gaussian_noise(0, .05);

    place_orders_(Side::buy, theo, spread_offset);
    place_orders_(Side::sell, theo, spread_offset);
}
} // namespace nutc::exchange
