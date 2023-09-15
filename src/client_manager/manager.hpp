#pragma once
// keep track of active users and account information
#include <glaze/glaze.hpp>

#include <string>
#include <unordered_map>

namespace nutc {
namespace manager {
struct Client {
    std::string uid;
    bool active;
};

class ClientManager {
public:
    void addClient(const std::string& uid);
    void setClientActive(const std::string& uid);
    void initialize_from_firebase(const glz::json_t::object_t& users);
    std::vector<Client> getClients(bool active) const;

private:
    std::unordered_map<std::string, Client> clients;
};

} // namespace manager
} // namespace nutc
