#include "bot_trader.hpp"

#include <cassert>

namespace nutc {
namespace traders {

void
BotTrader::process_position_change(limit_order order)
{
    assert(order.ticker == TICKER);

    double total_cap = order.price * order.quantity;
    if (order.side == util::Side::buy) {
        modify_long_capital(total_cap);
        modify_open_bids(order.quantity);
    }
    else {
        modify_short_capital(total_cap);
        modify_open_asks(order.quantity);
    }
}

} // namespace traders
} // namespace nutc
