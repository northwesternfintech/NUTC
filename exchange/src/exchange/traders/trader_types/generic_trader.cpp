#include "generic_trader.hpp"

#include "shared/types/decimal.hpp"

namespace nutc::exchange {

void
GenericTrader::notify_match(shared::position match)
{
    shared::decimal_price total_cap = match.price * match.quantity;
    if (match.side == shared::Side::buy) {
        modify_holdings(match.ticker, match.quantity);
        modify_capital(total_cap * -1.0);
    }
    else {
        modify_holdings(match.ticker, -match.quantity);
        modify_capital(total_cap);
    }
}

} // namespace nutc::exchange
