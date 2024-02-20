#include "trader_manager.hpp"

namespace nutc {
namespace manager {

std::optional<messages::SIDE>
ClientManager::validate_match(const messages::Match& match) const
{
    double trade_value = match.price * match.quantity;
    double remaining_capital = get_trader(match.buyer_id)->get_capital() - trade_value;

    bool insufficient_holdings =
        get_trader(match.seller_id)->get_holdings(match.ticker) - match.quantity < 0;

    if (remaining_capital < 0) [[unlikely]]
        return messages::SIDE::BUY;

    if (insufficient_holdings) [[unlikely]]
        return messages::SIDE::SELL;

    return std::nullopt;
}

} // namespace manager
} // namespace nutc
