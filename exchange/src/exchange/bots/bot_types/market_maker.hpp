#pragma once
#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/theo/random_noise_generator.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "shared/types/decimal_price.hpp"

#include <sys/types.h>

namespace nutc::bots {
using util::decimal_price;

struct price_level {
    const decimal_price PRICE_DELTA;
    const double QUANTITY_FACTOR;

    consteval price_level(decimal_price price_delta, double quantity_factor) :
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

/**
 * No thread safety - do not run functions on multiple threads
 */
template <typename RandomNoiseGenerator = stochastic::RandomNoiseGenerator>
class MarketMakerBot : public traders::BotTrader {
public:
    MarketMakerBot(util::Ticker ticker, decimal_price interest_limit) :
        BotTrader(ticker, interest_limit)
    {}

    void
    take_action(const shared_bot_state& state) override
    {
        decimal_price lean = calculate_lean_percent(state);
        decimal_price noised_theo =
            state.THEO - (lean * 15.0) + RandomNoiseGenerator::generate(0, .05);

        place_orders(util::Side::buy, noised_theo, 0.0);
        place_orders(util::Side::sell, noised_theo, 0.0);
    }

    const std::string&
    get_type() const final
    {
        static const std::string TYPE = "MARKET_MAKER";
        return TYPE;
    }

private:
    static decimal_price
    calculate_lean_percent(const shared_bot_state& state)
    {
        if (state.CUMULATIVE_INTEREST_LIMIT == 0) [[unlikely]]
            return 0.0;

        auto lean_price = state.MIDPRICE * state.CUMULATIVE_QUANTITY_HELD;
        return lean_price / state.CUMULATIVE_INTEREST_LIMIT;
    }

    void
    place_orders(util::Side side, decimal_price theo, decimal_price spread_offset)
    {
        // Approximation
        double total_quantity{compute_capital_tolerance_() / (theo + spread_offset)};

        // Placing orders on both sides
        total_quantity /= 2;

        for (const auto& [price_delta, quantity_factor] : PRICE_LEVELS) {
            decimal_price price = (side == util::Side::buy)
                                      ? theo - price_delta - spread_offset
                                      : theo + price_delta + spread_offset;

            if (price > 0.0) [[likely]]
                add_limit_order(side, total_quantity * quantity_factor, price, true);
        }
    }
};

} // namespace nutc::bots
