#pragma once

#include "shared/types/decimal_price.hpp"

namespace nutc::bots {
struct shared_bot_state {
    const util::decimal_price MIDPRICE;
    const util::decimal_price THEO;
    const double REALIZED_VOLATILITY;
    const util::decimal_price CUMULATIVE_INTEREST_LIMIT;
    const double CUMULATIVE_QUANTITY_HELD;
};
} // namespace nutc::bots
