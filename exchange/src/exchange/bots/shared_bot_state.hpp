#pragma once

namespace nutc::bots {
struct shared_bot_state {
    const double MIDPRICE;
    const double THEO;
    const double REALIZED_VOLATILITY;
    const double CUMULATIVE_INTEREST_LIMIT;
    const double CUMULATIVE_QUANTITY_HELD;
};
} // namespace nutc::bots
