#include "manager.hpp"

namespace nutc {
namespace accounts {
void
ClientManager::addClient(const std::string& uid, bool active)
{
    Client client{uid};
    active ? activeClients.push_back(client) : inactiveClients.push_back(client);
}

std::vector<Client>
ClientManager::getActiveClients()
{
    return activeClients;
}

std::vector<Client>
ClientManager::getInactiveClients()
{
    return inactiveClients;
}
} // namespace accounts
} // namespace nutc
