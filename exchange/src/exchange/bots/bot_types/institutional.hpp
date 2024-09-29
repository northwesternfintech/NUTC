#pragma once
#include "common/types/decimal.hpp"
#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"

#include <sys/types.h>

#include <random>

namespace nutc::exchange {
using common::decimal_price;

/**
 * No thread safety - do not run functions on multiple threads
 */
class InstitutionalBot : public BotTrader {
    std::mt19937 gen_{std::random_device{}()};
    std::uniform_real_distribution<float> dist_{0.0f, 100.0f};

    int num_ = 0;

public:
    InstitutionalBot(common::Ticker ticker, double interest_limit) :
        BotTrader(ticker, interest_limit)
    {}

    void take_action(const shared_bot_state& state) override;

    const std::string&
    get_type() const final
    {
        static const std::string TYPE = "INSTITUTIONAL_BOT";
        return TYPE;
    }
};

} // namespace nutc::exchange
