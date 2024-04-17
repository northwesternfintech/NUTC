#pragma once

#include "exchange/tickers/engine/order_storage.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"

#include <cstdint>

#include <optional>
#include <random>

namespace nutc {

namespace bots {

class RetailBot : public BotTrader {
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

    std::optional<matching::StoredOrder>
    take_action(double current, double theo, uint64_t current_tick);

private:
    const double AGGRESSIVENESS;
};

} // namespace bots

} // namespace nutc
