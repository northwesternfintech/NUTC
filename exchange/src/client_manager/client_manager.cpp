#include "client_manager.hpp"

namespace nutc {
namespace manager {

void
ClientManager::set_client_pid(const std::string& id, pid_t pid)
{
    if (!user_exists(id))
        return;

    clients[id].pid = pid;
}

void
ClientManager::modify_holdings(
    const std::string& id, const std::string& ticker, float change_in_holdings
)
{
    if (!user_exists(id))
        return;

    if (!user_holds_stock(id, ticker)) {
        clients[id].holdings[ticker] = change_in_holdings;
        return;
    }

    clients[id].holdings[ticker] += change_in_holdings;
}

std::optional<messages::SIDE>
ClientManager::validate_match(const messages::Match& match) const
{
    float trade_value = match.price * match.quantity;
    float remaining_capital = get_capital(match.buyer_id) - trade_value;
    bool insufficient_holdings =
        get_holdings(match.seller_id, match.ticker) - match.quantity < 0;

    if (remaining_capital < 0) [[unlikely]]
        return messages::SIDE::BUY;

    if (match.seller_id != "SIMULATED" && insufficient_holdings) [[unlikely]]
        return messages::SIDE::SELL;

    return std::nullopt;
}

void
ClientManager::initialize_from_firebase(const glz::json_t::object_t& users)
{
    for (auto& [id, user] : users) {
        if (!user.contains("latestAlgoId"))
            continue;
        add_client(id, user["latestAlgoId"].get<std::string>(), false);
    }
}

void
ClientManager::set_active(const std::string& id)
{
    if (!user_exists(id))
        return;

    clients[id].active = true;
}

} // namespace manager
} // namespace nutc
