#pragma once

#include "exchange/traders/trader_types/bot_trader.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <cmath>

#include <deque>
#include <numeric>
#include <random>
#include <vector>

namespace nutc {

namespace bots {

class BollingerBot : public traders::BotTrader {
    std::random_device rd{};
    std::mt19937 gen{rd()}; // not really sure what this does/if it's necessary
                            // auto generate aggresiveness?

    std::deque<double> price_history{};
    const std::size_t sma_period;
    const double multiplier;

    double current_sma = 0.0;
    double current_variance = 0.0;
    std::size_t num_data_points = 0;

public:
    BollingerBot(
        std::string ticker, double interest_limit, std::size_t period = 20,
        double aggressiveness = 2.0
    ) :
        BotTrader(std::move(ticker), interest_limit),
        sma_period(period), multiplier(aggressiveness)
    {}

    bool constexpr can_leverage() const override { return true; }

    [[nodiscard]] bool is_active() const override;

    std::optional<messages::market_order> take_action(double current, double theo);

private:
    void update_statistics(double new_price);
    std::pair<double, double> calculate_bollinger_bands();
};

} // namespace bots

} // namespace nutc
