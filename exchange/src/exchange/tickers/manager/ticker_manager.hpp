#pragma once

#include "exchange/bots/bot_container.hpp"
#include "exchange/tick_scheduler/tick_observer.hpp"
#include "exchange/tickers/engine/engine.hpp"
#include "exchange/tickers/engine/order_container.hpp"
#include "exchange/tickers/engine/order_storage.hpp"

#include <string>

namespace nutc {
namespace engine_manager {

using Engine = matching::Engine;

struct ticker_info {
    Engine engine;
    bots::BotContainer bot_container;
    matching::OrderContainer last_order_container;
    uint64_t num_matches{};

    ticker_info(
        std::string ticker, size_t ex_ticks, double starting_price, double order_fee
    ) :
        engine(ex_ticks, order_fee),
        bot_container(std::move(ticker), starting_price)
    {}
};

class EngineManager : public nutc::ticks::TickObserver {
    std::vector<matching::stored_match> matches_;
    std::unordered_map<std::string, ticker_info> engines_;
    EngineManager() = default;

public:
    double get_midprice(const std::string& ticker) const;
    const ticker_info& get_engine(const std::string& ticker) const;
    ticker_info& get_engine(const std::string& ticker);
    bool has_engine(const std::string& ticker) const;

    uint64_t
    get_num_matches(const std::string& ticker) const
    {
        assert(has_engine(ticker));
        return get_engine(ticker).num_matches;
    }

    bots::BotContainer&
    get_bot_container(const std::string& ticker)
    {
        return get_engine(ticker).bot_container;
    }

    size_t
    match_order(const matching::stored_order& order)
    {
        auto& engine = get_engine(order.ticker);
        std::vector<matching::stored_match> matches = engine.engine.match_order(order);
        engine.num_matches += matches.size();
        matches_.insert(matches_.end(), matches.begin(), matches.end());
        return matches.size();
    }

    void add_engine(const std::string& ticker, double starting_price);
    void add_engine(const std::string& ticker);

    void on_tick(uint64_t new_tick) override;

    // For testing
    void
    reset()
    {
        engines_.clear();
    }

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
