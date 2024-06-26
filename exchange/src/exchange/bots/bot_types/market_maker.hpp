#pragma once
#include "exchange/traders/trader_types/bot_trader.hpp"

#include <sys/types.h>

namespace nutc {
namespace bots {

// TODO for hardening: if price gets close to 0, quantity will get very high because we
// divide by price. Maybe something to think about?
/**
 * No thread safety - do not run functions on multiple threads
 */
class MarketMakerBot : public traders::BotTrader {
public:
    MarketMakerBot(util::Ticker ticker, double interest_limit) :
        BotTrader(ticker, interest_limit)
    {}

    bool constexpr can_leverage() const override { return true; }

    void take_action(double midprice, double theo) override;

    const std::string&
    get_type() const final
    {
        static const std::string TYPE = "MARKET_MAKER";
        return TYPE;
    }

private:
    static constexpr double avg_level_price(double new_theo);
};

} // namespace bots
} // namespace nutc
