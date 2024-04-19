#pragma once
#include "exchange/bots/bot_types/market_maker.hpp"
#include "exchange/bots/bot_types/retail.hpp"
#include "exchange/theo/brownian.hpp"
#include "exchange/tick_scheduler/tick_observer.hpp"
#include "exchange/tickers/engine/order_storage.hpp"

#include <cstdint>

namespace nutc {

namespace bots {

template <typename T>
concept HandledBotType =
    std::disjunction_v<std::is_same<T, RetailBot>, std::is_same<T, MarketMakerBot>>;

/**
 * @brief Container for all bots in the exchange for a given ticker
 */
class BotContainer : public ticks::TickObserver {
    const std::string TICKER;
    const double BROWNIAN_OFFSET;
    stochastic::BrownianMotion theo_generator_;

    std::unordered_map<std::string, const std::shared_ptr<RetailBot>> retail_bots_{};
    std::unordered_map<std::string, const std::shared_ptr<MarketMakerBot>>
        market_makers_{};

public:
    void on_tick(uint64_t) override;

    double
    get_theo() const
    {
        return BROWNIAN_OFFSET + theo_generator_.get_price();
    }

    const auto&
    get_market_makers() const
    {
        return market_makers_;
    }

    const auto&
    get_retail_traders() const
    {
        return retail_bots_;
    }

    std::vector<matching::stored_order>
    on_new_theo(double new_theo, double current, uint64_t current_tick);

    template <class BotType>
    void add_bots(double mean_capital, double stddev_capital, size_t num_bots)
    requires HandledBotType<BotType>;

    BotContainer() : BROWNIAN_OFFSET(0.0) {}

    explicit BotContainer(std::string ticker, double starting_price) :
        TICKER(std::move(ticker)), BROWNIAN_OFFSET(starting_price)
    {}

private:
    template <class BotType>
    void add_single_bot_(double starting_capital)
    requires HandledBotType<BotType>;
};
} // namespace bots
} // namespace nutc
