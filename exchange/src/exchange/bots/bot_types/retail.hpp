#pragma once

#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "shared/types/decimal.hpp"

#include <random>

namespace nutc::exchange {

class RetailBot : public BotTrader {
    std::mt19937 gen_{}; // NOLINT
    std::poisson_distribution<> poisson_dist_{};

    shared::decimal_price
    generate_aggresiveness_()
    {
        static std::normal_distribution<> dist{1000, 2000};
        return dist(gen_);
    }

public:
    RetailBot(shared::Ticker ticker, shared::decimal_price interest_limit) :
        BotTrader(ticker, interest_limit), AGGRESSIVENESS(generate_aggresiveness_())
    {}

    void take_action(const shared_bot_state& state) override;

    const std::string&
    get_type() const final
    {
        static const std::string TYPE = "RETAIL";
        return TYPE;
    }

private:
    const shared::decimal_price AGGRESSIVENESS;
};

} // namespace nutc::exchange
