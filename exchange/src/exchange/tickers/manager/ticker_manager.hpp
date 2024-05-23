#pragma once

#include "exchange/bots/bot_container.hpp"
#include "exchange/config/dynamic/config.hpp"
#include "exchange/tick_scheduler/tick_observer.hpp"
#include "exchange/tickers/engine/engine.hpp"
#include "exchange/tickers/engine/level_update_generator.hpp"
#include "exchange/tickers/engine/order_container.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

#include <string>

namespace nutc {
namespace engine_manager {

using Engine = matching::Engine;
using Orderbook = matching::OrderBook;

struct ticker_info {
    std::shared_ptr<matching::LevelUpdateGenerator> level_update_generator_;
    matching::OrderBook orderbook;

    Engine engine;
    bots::BotContainer bot_container;

    ticker_info(
        std::string ticker, double starting_price, double order_fee,
        std::vector<config::bot_config> config
    ) :
        level_update_generator_(std::make_shared<matching::LevelUpdateGenerator>()),
        orderbook(level_update_generator_), engine(order_fee),
        bot_container(std::move(ticker), starting_price, std::move(config))
    {}
};

class EngineManager : public nutc::ticks::TickObserver {
    std::vector<matching::stored_match> accum_matches_;
    std::unordered_map<std::string, ticker_info> engines_;
    size_t order_expiry_ticks;
    double order_fee;

public:
    EngineManager() :
        order_expiry_ticks(config::Config::get().constants().ORDER_EXPIRATION_TICKS),
        order_fee(config::Config::get().constants().ORDER_FEE)
    {}

    EngineManager(size_t order_expiry_ticks, double order_fee) :
        order_expiry_ticks(order_expiry_ticks), order_fee(order_fee)
    {}

    double get_midprice(const std::string& ticker) const;
    bool has_engine(const std::string& ticker) const;

    size_t match_order(const matching::stored_order& order);

    void add_engine(const config::ticker_config& config);
    void add_engine(const std::string& ticker);

    void on_tick(uint64_t new_tick) override;

    EngineManager(EngineManager const&) = delete;
    EngineManager operator=(EngineManager const&) = delete;
    EngineManager(EngineManager&&) = delete;
    EngineManager operator=(EngineManager&&) = delete;

private:
    std::vector<std::string> split_tick_updates_(const messages::tick_update& update);
    ticker_info& get_engine(const std::string& ticker);
    const ticker_info& get_engine(const std::string& ticker) const;
};
} // namespace engine_manager
} // namespace nutc
