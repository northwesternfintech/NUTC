#pragma once
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

    bool constexpr can_leverage() const override { return true; }

    void take_action(double, double theo, double variance) override;

    const std::string&
    get_type() const final
    {
        static const std::string TYPE = "MARKET_MAKER";
        return TYPE;
    }

private:
    static constexpr double avg_level_price(double new_theo, double offset);
};

} // namespace bots
} // namespace nutc
