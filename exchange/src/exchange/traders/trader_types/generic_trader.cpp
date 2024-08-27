#include "generic_trader.hpp"

#include "common/types/decimal.hpp"

namespace nutc::exchange {

void
GenericTrader::notify_match(common::position match)
{
    common::decimal_price total_cap = match.price * match.quantity;
    if (match.side == common::Side::buy) {
        modify_holdings(match.ticker, match.quantity);
        modify_capital(total_cap * -1.0);
    }
    else {
        modify_holdings(match.ticker, -(match.quantity));
        modify_capital(total_cap);
    }
}

} // namespace nutc::exchange
