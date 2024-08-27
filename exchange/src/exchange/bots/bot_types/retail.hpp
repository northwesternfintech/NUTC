#pragma once

#include "common/types/decimal.hpp"
#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"

#include <random>

namespace nutc::exchange {

class RetailBot : public BotTrader {
    std::mt19937 gen_{}; // NOLINT
    std::poisson_distribution<> poisson_dist_{};

    common::decimal_price
    generate_aggresiveness_()
    {
        static std::normal_distribution<> dist{1000, 2000};
        return dist(gen_);
    }

public:
    RetailBot(common::Ticker ticker, common::decimal_price interest_limit) :
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
    const common::decimal_price AGGRESSIVENESS;
};

} // namespace nutc::exchange
