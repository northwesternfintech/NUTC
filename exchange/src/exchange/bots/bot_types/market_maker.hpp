#pragma once
#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"

#include <sys/types.h>

#include <random>

namespace nutc {
namespace bots {

/**
 * No thread safety - do not run functions on multiple threads
 */
class MarketMakerBot : public traders::BotTrader {
    // TODO: parameterize
    static double
    gen_aggressiveness()
    {
        static std::mt19937 gen(std::random_device{}());
        static std::uniform_real_distribution<double> dist(0.7, 1.0);
        return dist(gen);
    }

    double aggressiveness = gen_aggressiveness();

public:
    MarketMakerBot(util::Ticker ticker, double interest_limit) :
        BotTrader(ticker, interest_limit)
    {}

    void take_action(const shared_bot_state& state) override;

    const std::string&
    get_type() const final
    {
        static const std::string TYPE = "MARKET_MAKER";
        return TYPE;
    }

    double calculate_lean(const shared_bot_state& state);

private:
    void place_orders(util::Side side, double theo, double spread_offset);
};

} // namespace bots
} // namespace nutc
