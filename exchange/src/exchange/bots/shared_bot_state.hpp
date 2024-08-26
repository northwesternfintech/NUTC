#pragma once

#include "shared/types/decimal.hpp"

namespace nutc::exchange {
struct shared_bot_state {
    const shared::decimal_price MIDPRICE;
    const shared::decimal_price THEO;
    const double REALIZED_VOLATILITY;
    const shared::decimal_price CUMULATIVE_INTEREST_LIMIT;
    const double CUMULATIVE_QUANTITY_HELD;
};
} // namespace nutc::exchange
