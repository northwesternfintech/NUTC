#include "manager.hpp"

namespace nutc {
namespace manager {

std::optional<messages::SIDE>
ClientManager::validateMatch(const messages::Match& match) const
{
    float trade_value = match.price * match.quantity;
    if (getCapital(match.buyer_uid) - trade_value < 0) {
        return messages::SIDE::BUY;
    }
    // TODO: holdings check
    return std::nullopt;
}

void
ClientManager::initialize_from_firebase(const glz::json_t::object_t& users)
{
    for (auto& [uid, _] : users) {
        addClient(uid);
    }
}

void
ClientManager::addClient(const std::string& uid)
{
    if (clients.find(uid) != clients.end()) {
        return;
    }
    clients[uid] = Client{uid, false, STARTING_CAPITAL};
}

float
ClientManager::modifyCapital(const std::string& uid, float change_in_capital)
{
    if (clients.find(uid) == clients.end()) {
        return 0;
    }
    clients[uid].capital_remaining += change_in_capital;
    return clients[uid].capital_remaining;
}

float
ClientManager::getCapital(const std::string& uid) const
{
    if (clients.find(uid) == clients.end()) {
        return 0;
    }
    return clients.at(uid).capital_remaining;
}

void
ClientManager::setClientActive(const std::string& uid)
{
    if (clients.find(uid) == clients.end()) {
        return;
    }
    clients[uid].active = true;
}

// inefficient but who cares
std::vector<Client>
ClientManager::getClients(bool active) const
{
    std::vector<Client> client_vec;
    for (auto& [uid, client] : clients) {
        if ((active && client.active) || (!active && !client.active)) {
            client_vec.push_back(client);
        }
    }
    return client_vec;
}
} // namespace manager
} // namespace nutc
