#include "market_maker.hpp"

#include "exchange/bots/shared_bot_state.hpp"
#include "shared/types/decimal_price.hpp"
#include "shared/util.hpp"

#include <algorithm>
#include <array>

using nutc::shared::Side;

namespace {
struct price_level {
    const nutc::shared::decimal_price PRICE_DELTA;
    const double QUANTITY_FACTOR;

    consteval price_level(
        nutc::shared::decimal_price price_delta, double quantity_factor
    ) : PRICE_DELTA(price_delta), QUANTITY_FACTOR(quantity_factor)
    {}
};

// TODO: parameterize
constexpr double BASE_SPREAD = 0.16;
constexpr uint8_t LEVELS = 3;
constexpr std::array<price_level, LEVELS> PRICE_LEVELS{
    price_level{BASE_SPREAD + .00, 1.0 / 2},
    price_level{BASE_SPREAD + .05, 1.0 / 3},
    price_level{BASE_SPREAD + .10, 1.0 / 6}
};

} // namespace

namespace nutc::exchange {

void
MarketMakerBot::place_orders(Side side, decimal_price theo, decimal_price spread_offset)
{
    // Approximation
    double total_quantity =
        double{compute_capital_tolerance_() / (theo + spread_offset)};

    // Placing orders on both sides
    total_quantity /= 2;

    for (const auto& [price_delta, quantity_factor] : PRICE_LEVELS) {
        decimal_price price = (side == Side::buy) ? theo - price_delta - spread_offset
                                                  : theo + price_delta + spread_offset;

        if (price <= 0.0) [[unlikely]]
            return;

        add_limit_order(side, total_quantity * quantity_factor, price, true);
    }
}

decimal_price
MarketMakerBot::calculate_lean_percent(const shared_bot_state& state)
{
    auto lean_price = state.MIDPRICE * state.CUMULATIVE_QUANTITY_HELD;
    return lean_price / state.CUMULATIVE_INTEREST_LIMIT;
}

// TODO: clean up
void
MarketMakerBot::take_action(const shared_bot_state& state)
{
    // decimal_price spread_offset = state.MIDPRICE * (1.0 / 600 +
    // state.REALIZED_VOLATILITY); spread_offset *= aggressiveness; spread_offset =
    // std::min(spread_offset, 1.0); spread_offset = std::max(spread_offset, -1.0);
    //
    // TODO: adding market impact to the spread is very challenging and will be
    // continued later
    decimal_price spread_offset = 0.0;

    decimal_price lean = calculate_lean_percent(state);
    decimal_price theo = state.THEO - (lean * 15.0) + generate_gaussian_noise(0, .05);

    place_orders(Side::buy, theo, spread_offset);
    place_orders(Side::sell, theo, spread_offset);
}
} // namespace nutc::exchange
