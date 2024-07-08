#include "generic_trader.hpp"

namespace nutc {
namespace traders {

void
GenericTrader::process_order_match(util::position match)
{
    double total_cap = double{match.price} * match.quantity;
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
