#include "market_maker.hpp"

#include <algorithm>
#include <array>

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

constexpr double
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

void
MarketMakerBot::take_action(double, double real_theo)
{
    double theo = real_theo + generate_gaussian_noise(0, .1);

    double capital_tolerance = compute_capital_tolerance_();
    double average_price = avg_level_price(theo);
    double total_quantity = capital_tolerance / average_price;

    auto new_buy_order = [&](const price_level& level) {
        double price = theo + level.PRICE_DELTA;
        double quantity = total_quantity * level.QUANTITY_FACTOR;
        add_order(util::Side::buy, price, quantity, true);
    };
    auto new_sell_order = [&](const price_level& level) {
        double price = theo + level.PRICE_DELTA;
        double quantity = total_quantity * level.QUANTITY_FACTOR;
        add_order(util::Side::sell, price, quantity, true);
    };

    std::ranges::for_each(BID_LEVELS, new_buy_order);
    std::ranges::for_each(ASK_LEVELS, new_sell_order);
}
} // namespace bots
} // namespace nutc
