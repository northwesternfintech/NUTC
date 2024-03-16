#pragma once

#include "exchange/bots/bot_container.hpp"
#include "exchange/config.h"
#include "exchange/tick_manager/tick_observer.hpp"
#include "exchange/tickers/engine/engine.hpp"
#include "exchange/tickers/engine/order_storage.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"

#include <memory>
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

            auto process_order_change = [](const matching::StoredOrder& stored_order,
                                           bool opened_order) {
                if (stored_order.trader->get_type() != manager::TraderType::BOT)
                    return;
                auto bot =
                    std::static_pointer_cast<bots::BotTrader>(stored_order.trader);
                double total_cap = stored_order.price * stored_order.quantity
                                   * (opened_order ? 1 : -1);
                int order_change = opened_order ? 1 : -1;
                if (stored_order.side == messages::SIDE::BUY) {
                    bot->modify_long_capital(total_cap);
                    bot->modify_open_bids(order_change);
                }
                else {
                    bot->modify_short_capital(total_cap);
                    bot->modify_open_asks(order_change);
                }
            };

            std::for_each(
                added.begin(), added.end(),
                [&](const matching::StoredOrder& order) {
                    process_order_change(order, true);
                }
            );

            std::for_each(
                removed.begin(), removed.end(),
                [&](const matching::StoredOrder& order) {
                    process_order_change(order, false);
                }
            );

            for (Match& order : matched) {
                // TODO(stevenewald): check if bot beforehand?
                bot_containers_.at(ticker).process_order_match(order);
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
