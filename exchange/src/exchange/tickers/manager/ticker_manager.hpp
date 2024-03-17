#pragma once

#include "exchange/bots/bot_container.hpp"
#include "exchange/config.h"
#include "exchange/tick_manager/tick_observer.hpp"
#include "exchange/tickers/engine/engine.hpp"
#include "exchange/tickers/engine/order_storage.hpp"
#include "shared/util.hpp"

#include <string>

namespace nutc {
namespace engine_manager {

class EngineManager : public nutc::ticks::TickObserver {
public:
    matching::Engine& get_engine(const std::string& ticker);
    bool has_engine(const std::string& ticker) const;

    bots::BotContainer&
    get_bot_container(const std::string& ticker)
    {
        return bot_containers_.at(ticker);
    }

    void add_engine(const std::string& ticker, double starting_price);
    void add_engine(const std::string& ticker);

    void
    on_tick(uint64_t new_tick) override
    {
        if (new_tick < ORDER_EXPIRATION_TIME)
            return;
        for (auto& [ticker, engine] : engines_) {
            auto [removed, added, matched] =
                engine.on_tick(new_tick, ORDER_EXPIRATION_TIME);

            for (const auto& order : added) {
                order.trader->process_order_add(
                    order.ticker, order.side, order.price, order.quantity
                );
            }

            for (const auto& order : removed) {
                order.trader->process_order_expiration(
                    order.ticker, order.side, order.price, order.quantity
                );
            }

            for (const auto& match : matched) {
                match.buyer->process_order_match(
                    match.ticker, messages::SIDE::BUY, match.price, match.quantity
                );
                match.seller->process_order_match(
                    match.ticker, messages::SIDE::SELL, match.price, match.quantity
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
    void set_initial_price_(const std::string& ticker, double price);

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
