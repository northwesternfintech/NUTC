#pragma once
// keep track of active users and account information
#include <glaze/glaze.hpp>
#include "config.h"

#include <string>
#include <unordered_map>

namespace nutc {
/**
 * @brief Handles client tracking and capital management
 * Keeps track of all clients, whether they're active, and their capital
 */
namespace manager {
struct Client {
    std::string uid;
    bool active;
    float capital_remaining;
};

class ClientManager {
public:
    void addClient(const std::string& uid);
    void setClientActive(const std::string& uid);
    void initialize_from_firebase(const glz::json_t::object_t& users);
    float modifyCapital(const std::string& uid, float change_in_capital);
    float getCapital(const std::string& uid);
    std::vector<Client> getClients(bool active) const;

private:
    std::unordered_map<std::string, Client> clients;
};

} // namespace manager
} // namespace nutc
