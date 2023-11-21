#include "client_manager.hpp"

namespace nutc {
namespace manager {

inline bool
ClientManager::user_exists(const std::string& uid) const
{
    return clients.find(uid) != clients.end();
}

inline bool
ClientManager::user_holds_stock(const std::string& uid, const std::string& ticker) const
{
    if (!user_exists(uid)) [[unlikely]]
        return false;

    return clients.at(uid).holdings.find(ticker) != clients.at(uid).holdings.end();
}

float
ClientManager::get_holdings(const std::string& uid, const std::string& ticker) const
{
    if (!user_holds_stock(uid, ticker))
        return 0.0f;

    return clients.at(uid).holdings.at(ticker);
}

void
ClientManager::modify_holdings(
    const std::string& uid, const std::string& ticker, float change_in_holdings
)
{
    if (!user_exists(uid))
        return;

    if (!user_holds_stock(uid, ticker)) {
        clients[uid].holdings[ticker] = change_in_holdings;
        return;
    }

    clients[uid].holdings[ticker] += change_in_holdings;
}

std::optional<messages::SIDE>
ClientManager::validate_match(const messages::Match& match) const
{
    float trade_value = match.price * match.quantity;
    bool insufficient_capital = get_capital(match.buyer_uid) - trade_value < 0;
    bool insufficient_holdings =
        get_holdings(match.seller_uid, match.ticker) - match.quantity < 0;

    if (insufficient_capital) [[unlikely]]
        return messages::SIDE::BUY;

    if (match.seller_uid != "SIMULATED" && insufficient_holdings) [[unlikely]]
        return messages::SIDE::SELL;

    return std::nullopt;
}

void
ClientManager::initialize_from_firebase(const glz::json_t::object_t& users)
{
    for (auto& [uid, user] : users) {
        if (!user.contains("latestAlgoId"))
            continue;
        add_client(uid, user["latestAlgoId"].get<std::string>(), false);
    }
}

void
ClientManager::add_client(
    const std::string& uid, const std::string& algo_id, bool is_local_algo
)
{
    clients[uid] = Client{uid, algo_id, false, is_local_algo, STARTING_CAPITAL, {}};
}

void
ClientManager::add_client(const std::string& uid, const std::string& algo_id)
{
    clients[uid] = Client{uid, algo_id, false, false, STARTING_CAPITAL, {}};
}

void
ClientManager::modify_capital(const std::string& uid, float change_in_capital)
{
    if (!user_exists(uid))
        return;

    clients[uid].capital_remaining += change_in_capital;
}

float
ClientManager::get_capital(const std::string& uid) const
{
    if (!user_exists(uid))
        return 0.0f;

    return clients.at(uid).capital_remaining;
}

void
ClientManager::set_active(const std::string& uid)
{
    if (!user_exists(uid))
        return;

    clients[uid].active = true;
}

// inefficient but who cares
const std::unordered_map<std::string, Client>&
ClientManager::get_clients() const
{
    return clients;
}
} // namespace manager
} // namespace nutc
