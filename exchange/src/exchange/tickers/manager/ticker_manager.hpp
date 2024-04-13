#pragma once

#include "exchange/bots/bot_container.hpp"
#include "exchange/tick_manager/tick_observer.hpp"
#include "exchange/tickers/engine/engine.hpp"
#include "exchange/tickers/engine/order_container.hpp"
#include "exchange/tickers/engine/order_storage.hpp"

#include <string>

namespace nutc {
namespace engine_manager {

using Engine = matching::Engine;

class EngineManager : public nutc::ticks::TickObserver {
    // these should probably be combined into a single map. later problem :P
    std::map<std::string, Engine> engines_;
    std::vector<matching::StoredMatch> matches_;
    std::unordered_map<std::string, matching::OrderContainer> last_order_containers_;
    std::unordered_map<std::string, uint64_t> num_matches_;
    std::unordered_map<std::string, bots::BotContainer> bot_containers_;
    std::unordered_map<std::string, double> midprices_;
    EngineManager() = default;

public:
    double get_midprice(const std::string& ticker);
    Engine& get_engine(const std::string& ticker);
    bool has_engine(const std::string& ticker) const;

    uint64_t
    get_num_matches(const std::string& ticker) const
    {
        if (num_matches_.find(ticker) == num_matches_.end())
            return 0;
        return num_matches_.at(ticker);
    }

    bots::BotContainer&
    get_bot_container(const std::string& ticker)
    {
        return bot_containers_.at(ticker);
    }

    void
    match_order(const matching::StoredOrder& order)
    {
        std::vector<matching::StoredMatch> matches =
            get_engine(order.ticker).match_order(order);
        num_matches_[order.ticker] += matches.size();
        matches_.insert(matches_.end(), matches.begin(), matches.end());
    }

    void add_engine(const std::string& ticker, double starting_price);
    void add_engine(const std::string& ticker);

    void on_tick(uint64_t new_tick) override;

    // For testing
    void
    reset()
    {
        engines_.clear();
        bot_containers_.clear();
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
