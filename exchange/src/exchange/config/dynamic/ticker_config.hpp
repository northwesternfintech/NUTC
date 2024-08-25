#pragma once

#include "shared/types/decimal_price.hpp"
#include "shared/types/ticker.hpp"

#include <vector>

namespace nutc::exchange {

enum class BotType { market_maker = 0, retail = 1 };

struct bot_config {
    const BotType TYPE;
    const size_t NUM_BOTS;
    const shared::decimal_price AVERAGE_CAPITAL;
    const shared::decimal_price STD_DEV_CAPITAL;
};

struct ticker_config {
    const shared::Ticker TICKER;
    const shared::decimal_price STARTING_PRICE;

    const std::vector<bot_config> BOTS;
};
} // namespace nutc::exchange
