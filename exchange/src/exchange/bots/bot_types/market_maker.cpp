#include "market_maker.hpp"

#include "exchange/bots/shared_bot_state.hpp"
#include "shared/types/decimal_price.hpp"
#include "shared/util.hpp"

#include <algorithm>
#include <array>

using nutc::util::Side;

namespace {
struct price_level {
    const double PRICE_DELTA;
    const double QUANTITY_FACTOR;

    consteval price_level(double price_delta, double quantity_factor) :
        PRICE_DELTA(price_delta), QUANTITY_FACTOR(quantity_factor)
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

namespace nutc {
namespace bots {

void
MarketMakerBot::place_orders(Side side, double theo, double spread_offset)
{
    // Approximation
    double total_quantity = compute_capital_tolerance_() / (theo + spread_offset);

    // Placing orders on both sides
    total_quantity /= 2;

    for (const auto& [price_delta, quantity_factor] : PRICE_LEVELS) {
        double price = (side == Side::buy) ? theo - price_delta - spread_offset
                                           : theo + price_delta + spread_offset;

        if (price <= 0) [[unlikely]]
            return;

        double quantity = total_quantity * quantity_factor;
        add_limit_order(side, price, quantity, true);
    }
}

double
MarketMakerBot::calculate_lean(const shared_bot_state& state)
{
    double lean_price = state.CUMULATIVE_QUANTITY_HELD * state.MIDPRICE;
    return lean_price / state.CUMULATIVE_INTEREST_LIMIT;
}

// TODO: clean up
void
MarketMakerBot::take_action(const shared_bot_state& state)
{
    double theo = state.THEO + generate_gaussian_noise(0, .05);

    double spread_offset = state.MIDPRICE * (1.0 / 600 + state.REALIZED_VOLATILITY);
    spread_offset *= aggressiveness;
    spread_offset = std::min(spread_offset, 1.0);
    spread_offset = std::max(spread_offset, -1.0);

    // TODO: adding market impact to the spread is very challenging and will be
    // continued later
    spread_offset = 0;

    double lean_pcnt = calculate_lean(state);
    theo -= (lean_pcnt * 15);

    place_orders(Side::buy, theo, spread_offset);
    place_orders(Side::sell, theo, spread_offset);
}
} // namespace bots
} // namespace nutc
