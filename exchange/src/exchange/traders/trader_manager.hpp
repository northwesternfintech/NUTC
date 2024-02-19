#pragma once
// keep track of active users and account information
#include "shared/messages_exchange_to_wrapper.hpp"
#include "trader_types.hpp"

#include <glaze/glaze.hpp>

#include <optional>
#include <string>
#include <unordered_map>

namespace nutc {
namespace manager {

class ClientManager {
    std::unordered_map<std::string, std::unique_ptr<GenericTrader>> traders_;

public:
    void
    add_remote_trader(const std::string& user_id, const std::string& algo_id)
    {
        traders_.emplace(user_id, std::make_unique<RemoteTrader>(user_id, algo_id));
    }

    void
    add_local_trader(const std::string& algo_id)
    {
        traders_.emplace(algo_id, std::make_unique<LocalTrader>(algo_id));
    }

    // returns internal id
    [[nodiscard]] std::string
    add_bot_trader()
    {
        std::unique_ptr<GenericTrader> bot = std::make_unique<BotTrader>();
        std::string bot_id = bot->get_id();
        traders_.emplace(bot_id, std::move(bot));
        return bot_id;
    }

    [[nodiscard]] const std::unique_ptr<GenericTrader>&
    get_trader(const std::string& trader_id) const
    {
        assert(user_exists_(trader_id));
        return traders_.at(trader_id);
    }

    [[nodiscard]] std::unique_ptr<GenericTrader>&
    get_trader(const std::string& trader_id)
    {
        assert(user_exists_(trader_id));
        return traders_.at(trader_id);
    }

    std::unordered_map<std::string, std::unique_ptr<GenericTrader>>&
    get_traders()
    {
        return traders_;
    }

    const std::unordered_map<std::string, std::unique_ptr<GenericTrader>>&
    get_traders() const
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
