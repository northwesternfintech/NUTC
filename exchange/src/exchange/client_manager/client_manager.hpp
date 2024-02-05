#pragma once
// keep track of active users and account information
#include "exchange/config.h"
#include "shared/messages_exchange_to_wrapper.hpp"

#include <glaze/glaze.hpp>

#include <optional>
#include <string>
#include <unordered_map>

namespace nutc {
/**
 * @brief Handles client tracking and capital management
 * Keeps track of all clients, whether they're active, and their capital
 */
namespace manager {

// Bots are not spawned in
enum class ClientLocation { LOCAL, REMOTE, BOT };

struct client_t {
    std::string uid;
    pid_t pid;
    std::string algo_id;
    bool active;
    ClientLocation algo_location;
    float capital_remaining;
    std::unordered_map<std::string, float> holdings;
};

class ClientManager {
    std::unordered_map<std::string, client_t> clients_;

public:
    void
    add_client(
        const std::string& user_id, const std::string& algo_id,
        ClientLocation algo_location
    )
    {
        clients_[user_id] =
            client_t{user_id, {}, algo_id, false, algo_location, STARTING_CAPITAL, {}};
    }

    void
    modify_capital(const std::string& user_id, float change_in_capital)
    {
        if (!user_exists_(user_id))
            return;

        clients_[user_id].capital_remaining += change_in_capital;
    }

    float
    get_capital(const std::string& user_id) const
    {
        if (!user_exists_(user_id))
            return 0.0f;

        return clients_.at(user_id).capital_remaining;
    }

    void set_client_test(const std::string& user_id);
    void set_client_pid(const std::string& user_id, pid_t pid);
    void initialize_from_firebase(const glz::json_t::object_t& users);
    void set_active(const std::string& user_id);

    float
    get_holdings(const std::string& user_id, const std::string& ticker) const
    {
        if (!user_holds_stock_(user_id, ticker))
            return 0.0f;

        return clients_.at(user_id).holdings.at(ticker);
    }

    inline const std::unordered_map<std::string, client_t>&
    get_clients() const
    {
        return clients_;
    }

    void modify_holdings(
        const std::string& user_id, const std::string& ticker, float change_in_holdings
    );

    [[nodiscard]] std::optional<messages::SIDE>
    validate_match(const messages::Match& match) const;

private:
    bool
    user_exists_(const std::string& user_id) const
    {
        return clients_.contains(user_id);
    }

    bool
    user_holds_stock_(const std::string& user_id, const std::string& ticker) const
    {
        if (!user_exists_(user_id)) [[unlikely]]
            return false;

        return clients_.at(user_id).holdings.contains(ticker);
    }

    ClientManager() = default;

public:
    // Singleton
    static ClientManager&
    get_instance()
    {
        static ClientManager instance;
        return instance;
    }

    ClientManager(const ClientManager&) = delete;
    ClientManager(ClientManager&&) = delete;
    ClientManager& operator=(const ClientManager&) = delete;
    ClientManager& operator=(ClientManager&&) = delete;
};

} // namespace manager
} // namespace nutc

/* I don't think we should need this? It was already here and I don't want to break
anything
/// \cond
template <>
struct glz::meta<nutc::manager::client_t> {
    using t = nutc::manager::client_t;
    static constexpr auto value = object( // NOLINT (*)
        "uid", &t::uid, "active", &t::active, "is_local_algo", &t::is_local_algo,
        "capital_remaining", &t::capital_remaining, "holdings", &t::holdings
    );
};

/// \cond
template <>
struct glz::meta<nutc::manager::ClientManager> {
    static constexpr auto value = object( // NOLINT (*)
        "clients", [](auto&& self) { return self.get_clients(); }
    );
};
*/
