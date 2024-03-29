#include "trader_manager.hpp"

#include "exchange/tickers/engine/order_storage.hpp"

namespace nutc {
namespace manager {

std::optional<messages::SIDE>
TraderManager::validate_match(const matching::StoredMatch& match)
{
    double trade_value = match.price * match.quantity;
    double remaining_capital = match.buyer->get_capital() - trade_value;

    bool insufficient_holdings =
        !match.seller->can_leverage()
        && match.seller->get_holdings(match.ticker) - match.quantity < 0;

    if (!match.buyer->can_leverage() && remaining_capital < 0) [[unlikely]]
        return messages::SIDE::BUY;

    if (insufficient_holdings) [[unlikely]]
        return messages::SIDE::SELL;

    return std::nullopt;
}

} // namespace manager
} // namespace nutc
