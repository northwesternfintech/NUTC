#pragma once
// keep track of active users and account information
#include "config.h"
#include "utils/messages.hpp"

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

struct client {
    std::string uid;
    pid_t pid;
    std::string algo_id;
    bool active;
    bool is_local_algo;
    float capital_remaining;
    std::unordered_map<std::string, float> holdings;
};

class ClientManager {
    std::unordered_map<std::string, client> clients_;

public:
    void
    add_client(
        const std::string& user_id, const std::string& algo_id, bool is_local_algo
    )
    {
        clients_[user_id] =
            client{user_id, {}, algo_id, false, is_local_algo, STARTING_CAPITAL, {}};
    }

    void
    add_client(const std::string& user_id, const std::string& algo_id)
    {
        clients_[user_id] =
            client{user_id, {}, algo_id, false, false, STARTING_CAPITAL, {}};
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

    inline const std::unordered_map<std::string, client>&
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
};

} // namespace manager
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::manager::client> {
    using t = nutc::manager::client;
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
