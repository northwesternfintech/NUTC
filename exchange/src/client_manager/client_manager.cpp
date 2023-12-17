#include "client_manager.hpp"

namespace nutc {
namespace manager {

bool
ClientManager::user_exists(const std::string& id) const
{
    return clients.find(id) != clients.end();
}

void
ClientManager::set_client_pid(const std::string& id, pid_t pid)
{
    if (!user_exists(id))
        return;

    clients[id].pid = pid;
}

bool
ClientManager::user_holds_stock(const std::string& id, const std::string& ticker) const
{
    if (!user_exists(id)) [[unlikely]]
        return false;

    return clients.at(id).holdings.find(ticker) != clients.at(id).holdings.end();
}

float
ClientManager::get_holdings(const std::string& id, const std::string& ticker) const
{
    if (!user_holds_stock(id, ticker))
        return 0.0f;

    return clients.at(id).holdings.at(ticker);
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
    bool insufficient_capital = get_capital(match.buyer_id) - trade_value < 0;
    bool insufficient_holdings =
        get_holdings(match.seller_id, match.ticker) - match.quantity < 0;

    if (insufficient_capital) [[unlikely]]
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
ClientManager::add_client(
    const std::string& id, const std::string& algo_id, bool is_local_algo
)
{
    clients[id] = Client{id, {}, algo_id, false, is_local_algo, STARTING_CAPITAL, {}};
}

void
ClientManager::add_client(const std::string& id, const std::string& algo_id)
{
    clients[id] = Client{id, {}, algo_id, false, false, STARTING_CAPITAL, {}};
}

void
ClientManager::modify_capital(const std::string& id, float change_in_capital)
{
    if (!user_exists(id))
        return;

    clients[id].capital_remaining += change_in_capital;
}

float
ClientManager::get_capital(const std::string& id) const
{
    if (!user_exists(id))
        return 0.0f;

    return clients.at(id).capital_remaining;
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
