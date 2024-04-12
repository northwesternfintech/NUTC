#pragma once
#include "exchange/bots/bot_types/market_maker.hpp"
#include "exchange/bots/bot_types/retail.hpp"
#include "exchange/theo/brownian.hpp"
#include "exchange/tick_manager/tick_observer.hpp"
#include "exchange/tickers/engine/order_storage.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <cstdint>

namespace nutc {

namespace bots {
using MarketOrder = messages::MarketOrder;
using Match = messages::Match;

template <typename T>
concept HandledBotType =
    std::disjunction_v<std::is_same<T, RetailBot>, std::is_same<T, MarketMakerBot>>;

class BotContainer : public ticks::TickObserver {
public:
    void on_tick(uint64_t) override;

    double
    get_theo() const
    {
        return brownian_offset_ + theo_generator_.get_price();
    }

    const std::unordered_map<std::string, const std::shared_ptr<MarketMakerBot>>&
    get_market_makers() const
    {
        return market_makers_;
    }

    const std::unordered_map<std::string, const std::shared_ptr<RetailBot>>&
    get_retail_traders() const
    {
        return retail_bots_;
    }

    std::vector<matching::StoredOrder>
    on_new_theo(double new_theo, double current, uint64_t current_tick);

    template <class BotType>
    void add_bots(double mean_capital, double stddev_capital, size_t num_bots)
    requires HandledBotType<BotType>;

    BotContainer() = default;

    explicit BotContainer(std::string ticker, double starting_price) :
        ticker_(std::move(ticker)), brownian_offset_(starting_price)
    {}

private:
    template <class BotType>
    void add_single_bot_(double starting_capital)
    requires HandledBotType<BotType>;

    std::unordered_map<std::string, const std::shared_ptr<RetailBot>> retail_bots_{};
    std::unordered_map<std::string, const std::shared_ptr<MarketMakerBot>>
        market_makers_{};
    std::string ticker_;

    stochastic::BrownianMotion theo_generator_{};
    double brownian_offset_ = 0.0;
};
} // namespace bots
} // namespace nutc
