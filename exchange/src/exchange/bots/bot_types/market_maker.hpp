#pragma once
#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "shared/types/decimal.hpp"

#include <sys/types.h>

#include <random>

namespace nutc::exchange {
using shared::decimal_price;

/**
 * No thread safety - do not run functions on multiple threads
 */
class MarketMakerBot : public BotTrader {
    // TODO: parameterize
    static float
    gen_aggressiveness()
    {
        static std::mt19937 gen(std::random_device{}());
        static std::uniform_real_distribution<float> dist(0.7f, 1.0);
        return dist(gen);
    }

    float aggressiveness = gen_aggressiveness();

public:
    MarketMakerBot(shared::Ticker ticker, double interest_limit) :
        BotTrader(ticker, interest_limit)
    {}

    void take_action(const shared_bot_state& state) override;

    const std::string&
    get_type() const final
    {
        static const std::string TYPE = "MARKET_MAKER";
        return TYPE;
    }

    decimal_price calculate_lean_percent(const shared_bot_state& state);

private:
    void
    place_orders(shared::Side side, decimal_price theo, decimal_price spread_offset);
};

} // namespace nutc::exchange
