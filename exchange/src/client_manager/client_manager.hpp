#pragma once
// keep track of active users and account information
#include "config.h"
#include "utils/messages.hpp"

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
    std::string algo_id;
    bool active;
    bool is_local_algo;
    float capital_remaining;
    std::unordered_map<std::string, float> holdings;
};

class ClientManager {
private:
    std::unordered_map<std::string, Client> clients;

public:
    void add_client(const std::string& uid, const std::string& algo_id);
    void
    add_client(const std::string& uid, const std::string& algo_id, bool is_local_algo);
    void initialize_from_firebase(const glz::json_t::object_t& users);
    void set_active(const std::string& uid);

    float get_capital(const std::string& uid) const;
    float get_holdings(const std::string& uid, const std::string& ticker) const;

    inline const std::unordered_map<std::string, Client>&
    get_clients() const
    {
        return clients;
    }

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

/// \cond
template <>
struct glz::meta<nutc::manager::Client> {
    using T = nutc::manager::Client;
    /* clang-format off */
    static constexpr auto value = object(
        "uid", &T::uid, 
        "active", &T::active, 
        "is_local_algo", &T::is_local_algo,
        "capital_remaining", &T::capital_remaining,
        "holdings", &T::holdings
    );
    /* clang-format */
};

/// \cond
template <>
struct glz::meta<nutc::manager::ClientManager> {
    using T = nutc::manager::ClientManager;
    /* clang-format off */
    static constexpr auto value = object(
        "clients", [](auto&& self) -> auto& { return self.get_clients(); }
    );
    /* clang-format on */
};
