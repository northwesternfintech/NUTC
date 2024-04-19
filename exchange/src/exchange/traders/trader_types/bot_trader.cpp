#include "bot_trader.hpp"

#include <cassert>

#include <string>

namespace nutc {
namespace traders {

void
BotTrader::process_order_expiration(
    const std::string& ticker, util::Side side, double price, double quantity
)
{
    assert(ticker == TICKER);

    double total_cap = price * quantity;
    if (side == util::Side::buy) {
        modify_long_capital(-total_cap);
        modify_open_bids(-quantity);
    }
    else {
        modify_short_capital(-total_cap);
        modify_open_asks(-quantity);
    }
}

} // namespace traders
} // namespace nutc
