#pragma once
// keep track of active users and account information
#include <string>

namespace nutc {
namespace accounts {
struct Client {
    std::string uid;
};

class ClientManager {
public:
    void addClient(const std::string& uid, bool active);
    std::vector<Client> getActiveClients();
    std::vector<Client> getInactiveClients();

private:
    std::vector<Client> activeClients;
    std::vector<Client> inactiveClients;
};

} // namespace accounts
} // namespace nutc
