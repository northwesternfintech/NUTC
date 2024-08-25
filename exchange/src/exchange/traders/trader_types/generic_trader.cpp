#include "generic_trader.hpp"

#include "shared/types/decimal_price.hpp"

namespace nutc {
namespace traders {

void
GenericTrader::notify_match(util::position match)
{
    util::decimal_price total_cap = match.price * match.quantity;
    if (match.side == util::Side::buy) {
        modify_holdings(match.ticker, match.quantity);
        modify_capital(total_cap * -1.0);
    }
    else {
        modify_holdings(match.ticker, -match.quantity);
        modify_capital(total_cap);
    }
}

} // namespace traders
} // namespace nutc
