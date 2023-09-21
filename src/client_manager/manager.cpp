#include "manager.hpp"

namespace nutc {
namespace manager {
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
    clients[uid] = Client{uid, false};
}

void
ClientManager::setClientActive(const std::string& uid)
{
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
