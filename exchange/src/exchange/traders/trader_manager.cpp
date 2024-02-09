#include "trader_manager.hpp"

namespace nutc {
namespace manager {

std::optional<messages::SIDE>
ClientManager::validate_match(const messages::Match& match) const
{
    float trade_value = match.price * match.quantity;
    float remaining_capital = get_capital(match.buyer_id) - trade_value;

    bool insufficient_holdings =
        get_holdings(match.seller_id, match.ticker) - match.quantity < 0;

    if (remaining_capital < 0) [[unlikely]]
        return messages::SIDE::BUY;

    if (insufficient_holdings) [[unlikely]]
        return messages::SIDE::SELL;

    return std::nullopt;
}

} // namespace manager
} // namespace nutc
