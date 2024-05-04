#include "bot_trader.hpp"

#include <cassert>

#include <iostream>
#include <string>

namespace nutc {
namespace traders {

void
BotTrader::process_order_expiration(
    const std::string& ticker, util::Side side, double price, double quantity
)
{
    GenericTrader::process_order_expiration(ticker, side, price, quantity);
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

void
BotTrader::process_order_match(
    const std::string& ticker, util::Side side, double price, double quantity
)
{
    GenericTrader::process_order_match(ticker, side, price, quantity);
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
