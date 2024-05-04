#pragma once

#include "exchange/traders/trader_types/bot_trader.hpp"

#include <random>

namespace nutc {

namespace bots {

class RetailBot : public traders::BotTrader {
    std::mt19937 gen_{}; // NOLINT
    std::poisson_distribution<> poisson_dist_{};

    double
    generate_aggresiveness_()
    {
        static std::normal_distribution<> dist{1000, 2000};
        return dist(gen_);
    }

public:
    RetailBot(std::string ticker, double interest_limit) :
        BotTrader(std::move(ticker), interest_limit),
        AGGRESSIVENESS(generate_aggresiveness_())
    {}

    [[nodiscard]] bool is_active() const override;

    void take_action(double midprice, double theo) override;

private:
    const double AGGRESSIVENESS;
};

} // namespace bots

} // namespace nutc
