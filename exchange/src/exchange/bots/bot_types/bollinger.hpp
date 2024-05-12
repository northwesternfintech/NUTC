#pragma once

#include "exchange/traders/trader_types/bot_trader.hpp"

#include <deque>

namespace nutc {

namespace bots {

class BollingerBot : public traders::BotTrader {
    std::deque<double> price_history{};
    const std::size_t sma_period;
    const double multiplier;

    double current_sma = 0.0;
    double current_variance = 0.0;

public:
    BollingerBot(
        std::string ticker, double interest_limit, std::size_t period = 20,
        double aggressiveness = 2.0
    ) :
        BotTrader(std::move(ticker), interest_limit),
        sma_period(period), multiplier(aggressiveness)
    {}

    bool constexpr can_leverage() const override { return true; }

    void take_action(double current, double theo) override;

private:
    void update_statistics(double new_price);
    std::pair<double, double> calculate_bollinger_bands() const;
};

} // namespace bots

} // namespace nutc
