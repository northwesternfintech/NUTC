#include "generic_trader.hpp"

namespace nutc {
namespace traders {

void
GenericTrader::process_order_match(
    const std::string& ticker, util::Side side, double price, double quantity
)
{
    double total_cap = price * quantity;
    if (side == util::Side::buy) {
        modify_holdings(ticker, quantity);
        modify_capital(-total_cap);
    }
    else {
        modify_holdings(ticker, -quantity);
        modify_capital(total_cap);
    }
}

} // namespace traders
} // namespace nutc
