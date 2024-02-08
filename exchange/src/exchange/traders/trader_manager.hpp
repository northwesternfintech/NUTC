#pragma once
// keep track of active users and account information
#include "exchange/config.h"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "trader_types.hpp"

#include <glaze/glaze.hpp>

#include <optional>
#include <string>
#include <unordered_map>

namespace nutc {
namespace manager {

using trader_t = std::variant<remote_trader_t, local_trader_t, bot_trader_t>;

class ClientManager {
    std::unordered_map<std::string, trader_t> traders_{};

public:
    [[nodiscard]] std::string
    add_client(const trader_t& trader)
    {
        std::string user_id =
            std::visit([](auto&& trader) { return trader.get_id(); }, trader);
        traders_.emplace(user_id, trader);
        return user_id;
    }

    [[nodiscard]] trader_t&
    get_client(const std::string& user_id)
    {
        assert(user_exists_(user_id));
        return traders_.at(user_id);
    }

    [[nodiscard]] float
    get_holdings(const std::string& user_id, const std::string& ticker) const
    {
        assert(user_exists_(user_id));
        return std::visit(
            [&](auto&& trader) { return trader.get_holdings(ticker); },
            traders_.at(user_id)
        );
    }

    [[nodiscard]] float
    get_capital(const std::string& user_id) const
    {
        assert(user_exists_(user_id));
        return std::visit(
            [&](auto&& trader) { return trader.get_capital(); }, traders_.at(user_id)
        );
    }

    [[nodiscard]] const trader_t&
    get_client_const(const std::string& user_id) const
    {
        assert(user_exists_(user_id));
        return traders_.at(user_id);
    }

    void initialize_from_firebase(const glz::json_t::object_t& users);

    std::unordered_map<std::string, trader_t>&
    get_clients()
    {
        return traders_;
    }

    const std::unordered_map<std::string, trader_t>&
    get_clients_const() const
    {
        return traders_;
    }

    [[nodiscard]] std::optional<messages::SIDE>
    validate_match(const messages::Match& match) const;

    // Primarily for testing
    void
    reset()
    {
        traders_.clear();
    }

private:
    bool
    user_exists_(const std::string& user_id) const
    {
        return traders_.contains(user_id);
    }

    ClientManager() = default;
    ~ClientManager() = default;

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
