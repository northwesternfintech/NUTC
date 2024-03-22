#pragma once

#include "exchange/bots/bot_container.hpp"
#include "exchange/logging.hpp"
#include "exchange/rabbitmq/publisher/RabbitMQPublisher.hpp"
#include "exchange/tick_manager/tick_observer.hpp"
#include "exchange/tickers/engine/engine.hpp"
#include "exchange/tickers/engine/level_update_generator.hpp"
#include "exchange/tickers/engine/order_container.hpp"
#include "exchange/tickers/engine/order_storage.hpp"
#include "exchange/traders/trader_manager.hpp"
#include "shared/util.hpp"

#include <string>

namespace nutc {
namespace engine_manager {

using Engine = matching::Engine;

class EngineManager : public nutc::ticks::TickObserver {
public:
    Engine& get_engine(const std::string& ticker);
    bool has_engine(const std::string& ticker) const;

    bots::BotContainer&
    get_bot_container(const std::string& ticker)
    {
        return bot_containers_.at(ticker);
    }

    void
    match_order(const MarketOrder& order)
    {
        std::vector<matching::StoredMatch> matches =
            get_engine(order.ticker).match_order(order);
        matches_.insert(matches_.end(), matches.begin(), matches.end());
    }

    void add_engine(const std::string& ticker, double starting_price);
    void add_engine(const std::string& ticker);

    void
    on_tick(uint64_t new_tick) override
    {
        for (auto& [ticker, engine] : engines_) {
            std::vector<matching::StoredOrder> expired_orders =
                engine.expire_old_orders(new_tick);

            for (const auto& order : expired_orders) {
                order.trader->process_order_expiration(
                    order.ticker, order.side, order.price, order.quantity
                );
            }

            // TODO: do this in a converter
            std::vector<Match> glz_matches;
            for (const auto& match : matches_) {
                match.buyer->process_order_match(
                    match.ticker, messages::SIDE::BUY, match.price, match.quantity
                );
                match.seller->process_order_match(
                    match.ticker, messages::SIDE::SELL, match.price, match.quantity
                );
                glz_matches.emplace_back(
                    match.ticker, match.side, match.price, match.quantity,
                    match.buyer->get_id(), match.seller->get_id()
                );
            }

            std::vector<ObUpdate> updates = matching::LevelUpdateGenerator::get_updates(
                ticker, last_order_containers[ticker], engine.get_order_container()
            );
            for (auto& update1 : updates) {
                log_i(
                    main, "{} {} {}", update1.ticker, update1.price, update1.quantity
                );
            }
            last_order_containers[ticker] = engine.get_order_container();

            rabbitmq::RabbitMQPublisher::broadcast_ob_updates(
                manager::TraderManager::get_instance(), updates
            );

            rabbitmq::RabbitMQPublisher::broadcast_matches(
                manager::TraderManager::get_instance(), glz_matches
            );
            matches_.clear();
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
    // these should probably be combined into a single map. later problem :P
    std::map<std::string, Engine> engines_;
    std::vector<matching::StoredMatch> matches_;
    std::unordered_map<std::string, matching::OrderContainer> last_order_containers;
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
