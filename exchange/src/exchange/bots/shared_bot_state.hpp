#pragma once

#include "common/types/decimal.hpp"

namespace nutc::exchange {
struct shared_bot_state {
    const common::decimal_price MIDPRICE;
    const common::decimal_price THEO;
    const common::decimal_quantity REALIZED_VOLATILITY;
    const common::decimal_price CUMULATIVE_INTEREST_LIMIT;
    const common::decimal_quantity CUMULATIVE_QUANTITY_HELD;
};
} // namespace nutc::exchange
