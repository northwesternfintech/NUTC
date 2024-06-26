#pragma once

#include "shared/ticker.hpp"

#include <vector>

namespace nutc {
namespace config {

enum class BotType { market_maker, retail };

struct bot_config {
    const BotType TYPE;
    const size_t NUM_BOTS;
    const double AVERAGE_CAPITAL;
    const double STD_DEV_CAPITAL;
};

struct ticker_config {
    const util::Ticker TICKER;
    const double STARTING_PRICE;

    const std::vector<bot_config> BOTS;
};
} // namespace config
} // namespace nutc
