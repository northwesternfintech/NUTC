#include "bot_trader.hpp"

#include <cassert>

#include <string>

namespace nutc {
namespace bots {

void
BotTrader::process_order_expiration(
    const std::string& ticker, messages::SIDE side, double price, double quantity
)
{
    // We should only get an order expiration for the sole ticker this bot trades on
    assert(ticker == TICKER);

    double total_cap = price * quantity;
    if (side == messages::SIDE::BUY) {
        modify_long_capital(-total_cap);
        modify_open_bids(-1);
    }
    else {
        modify_short_capital(-total_cap);
        modify_open_asks(-1);
    }
}

void
BotTrader::process_order_add(
    const std::string& ticker, messages::SIDE side, double price, double quantity
)
{
    // We should only get an order add for the sole ticker this bot trades on
    assert(ticker == TICKER);

    double total_cap = price * quantity;
    if (side == messages::SIDE::BUY) {
        modify_long_capital(total_cap);
        modify_open_bids(1);
    }
    else {
        modify_short_capital(total_cap);
        modify_open_asks(1);
    }
}
} // namespace bots
} // namespace nutc
