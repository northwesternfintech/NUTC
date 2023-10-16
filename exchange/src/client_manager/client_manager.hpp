#pragma once
// keep track of active users and account information
#include "config.h"
#include "util/messages.hpp"

#include <glaze/glaze.hpp>

#include <iostream>
#include <optional>
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
    std::unordered_map<std::string, float> holdings;
};

class ClientManager {
private:
    std::unordered_map<std::string, Client> clients;

public:
    void add_client(
        const std::string& uid, float capital = STARTING_CAPITAL, bool active = false
    );
    void initialize_from_firebase(const glz::json_t::object_t& users);
    void set_active(const std::string& uid);

    float get_capital(const std::string& uid) const;
    float get_holdings(const std::string& uid, const std::string& ticker) const;
    std::vector<Client> get_clients(bool active) const;

    void modify_capital(const std::string& uid, float change_in_capital);
    void modify_holdings(
        const std::string& uid, const std::string& ticker, float change_in_holdings
    );

    [[nodiscard]] std::optional<messages::SIDE>
    validate_match(const messages::Match& match) const;

private:
    bool user_exists(const std::string& uid) const;
    bool user_holds_stock(const std::string& uid, const std::string& ticker) const;
};

} // namespace manager
} // namespace nutc
