#pragma once

#include "exchange/bots/bot_container.hpp"
#include "exchange/config.h"
#include "exchange/tick_manager/tick_observer.hpp"
#include "exchange/tickers/engine/engine.hpp"

#include <optional>
#include <string>

using engine_ref_t = std::reference_wrapper<nutc::matching::Engine>;

namespace nutc {
namespace engine_manager {

class EngineManager : public nutc::ticks::TickObserver {
public:
    std::optional<engine_ref_t> get_engine(const std::string& ticker);

    bots::BotContainer&
    get_bot_container(const std::string& ticker)
    {
        return bot_containers_.at(ticker);
    }

    void add_engine(const std::string& ticker, float starting_price);
    void add_engine(const std::string& ticker);

    // deprecated?
    void add_initial_liquidity(const std::string& ticker, float quantity, float price);

    void
    on_tick(uint64_t new_tick) override
    {
        if (new_tick < ORDER_EXPIRATION_TIME)
            return;
        for (auto& [ticker, engine] : engines_) {
            auto [removed, added] = engine.on_tick(new_tick, ORDER_EXPIRATION_TIME);

            for (auto& order : removed) {
                if (order.client_id.find("BOT_") == std::string::npos)
                    continue;
                bot_containers_.at(order.ticker)
                    .process_order_expiration(
                        order.client_id, order.side, order.price * order.quantity
                    );
            }
            for (auto& order : added) {
                if (order.client_id.find("BOT_") == std::string::npos)
                    continue;
                bot_containers_.at(order.ticker)
                    .process_order_add(
                        order.client_id, order.side, order.price * order.quantity
                    );
            }
        }
    }

    // For testing
    void
    reset()
    {
        engines_.clear();
        bot_containers_.clear();
    }

private:
    std::map<std::string, matching::Engine> engines_;
    std::unordered_map<std::string, bots::BotContainer> bot_containers_;
    EngineManager() = default;
    void set_initial_price_(const std::string& ticker, float price);

public:
    // fuck it, everything's a singleton

    static EngineManager&
    get_instance()
    {
        static EngineManager instance;
        return instance;
    }

    EngineManager(EngineManager const&) = delete;
    EngineManager operator=(EngineManager const&) = delete;
    EngineManager(EngineManager&&) = delete;
    EngineManager operator=(EngineManager&&) = delete;
};
} // namespace engine_manager
} // namespace nutc
