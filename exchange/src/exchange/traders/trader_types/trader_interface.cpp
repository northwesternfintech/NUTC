#include "trader_interface.hpp"

namespace nutc {
namespace traders {

void
GenericTrader::process_order_match(market_order match)
{
    double total_cap = match.price * match.quantity;
    if (match.side == util::Side::buy) {
        modify_holdings(match.ticker, match.quantity);
        modify_capital(-total_cap);
    }
    else {
        modify_holdings(match.ticker, -match.quantity);
        modify_capital(total_cap);
    }
}

} // namespace traders
} // namespace nutc
