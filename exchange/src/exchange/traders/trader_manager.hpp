#pragma once
#include "exchange/bots/bot_types/market_maker.hpp"
#include "exchange/bots/bot_types/retail.hpp"
#include "exchange/tickers/engine/order_storage.hpp"
#include "trader_types/bot_trader.hpp"
#include "trader_types/local_trader.hpp"
#include "trader_types/remote_trader.hpp"

#include <glaze/glaze.hpp>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace nutc {
namespace manager {

class TraderManager {
    std::unordered_map<std::string, const std::shared_ptr<GenericTrader>> traders_;

public:
    void
    add_remote_trader(
        const std::string& user_id, const std::string& algo_id, double capital
    )
    {
        traders_.emplace(
            user_id, std::make_shared<RemoteTrader>(user_id, algo_id, capital)
        );
    }

    void
    add_local_trader(const std::string& algo_id, double capital)
    {
        traders_.emplace(algo_id, std::make_shared<LocalTrader>(algo_id, capital));
    }

    // FOR TESTING ONLY - THIS WILL HAVE NO ATTACHED REAL BOT
    std::string
    add_bot_trader(double capital)
    {
        std::shared_ptr<GenericTrader> bot =
            std::make_shared<bots::BotTrader>("", capital);
        std::string bot_id = bot->get_id();
        traders_.insert({bot_id, bot});
        return bot_id;
    }

    std::shared_ptr<bots::RetailBot>
    add_bot_trader(bots::RetailBot&& trader)
    {
        std::string bot_id = trader.get_id();
        std::shared_ptr<GenericTrader> bot =
            std::make_shared<bots::RetailBot>(std::move(trader));
        traders_.insert({bot_id, bot});
        return std::dynamic_pointer_cast<bots::RetailBot>(bot);
    }

    std::shared_ptr<bots::MarketMakerBot>
    add_bot_trader(bots::MarketMakerBot&& trader)
    {
        std::string bot_id = trader.get_id();
        std::shared_ptr<GenericTrader> bot =
            std::make_shared<bots::MarketMakerBot>(std::move(trader));
        traders_.insert({bot_id, bot});
        return std::dynamic_pointer_cast<bots::MarketMakerBot>(bot);
    }

    [[nodiscard]] std::shared_ptr<GenericTrader>
    get_trader(const std::string& trader_id) const
    {
        assert(user_exists_(trader_id));
        return traders_.at(trader_id);
    }

    std::unordered_map<std::string, const std::shared_ptr<GenericTrader>>&
    get_traders()
    {
        return traders_;
    }

    const std::unordered_map<std::string, const std::shared_ptr<GenericTrader>>&
    get_traders() const
    {
        return traders_;
    }

    [[nodiscard]] static std::optional<messages::SIDE>
    validate_match(const matching::StoredMatch& match);

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

    TraderManager() = default;
    ~TraderManager() = default;

public:
    // Singleton
    static TraderManager&
    get_instance()
    {
        static TraderManager instance;
        return instance;
    }

    TraderManager(const TraderManager&) = delete;
    TraderManager(TraderManager&&) = delete;
    TraderManager& operator=(const TraderManager&) = delete;
    TraderManager& operator=(TraderManager&&) = delete;
};

} // namespace manager
} // namespace nutc
