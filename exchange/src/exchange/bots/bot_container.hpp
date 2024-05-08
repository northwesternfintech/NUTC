#pragma once
#include "exchange/bots/bot_types/market_maker.hpp"
#include "exchange/bots/bot_types/retail.hpp"
#include "exchange/theo/brownian.hpp"
#include "exchange/tick_scheduler/tick_observer.hpp"

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
    stochastic::BrownianMotion theo_generator_;

    std::vector<std::shared_ptr<traders::BotTrader>> bots_{};

public:
    void on_tick(uint64_t) override;

    double
    get_theo() const
    {
        return theo_generator_.get_magnitude();
    }

    const auto&
    get_bots() const
    {
        return bots_;
    }

    void generate_orders(double midprice, double new_theo);

    template <class BotType>
    void add_bots(double mean_capital, double stddev_capital, size_t num_bots)
    requires HandledBotType<BotType>;

    BotContainer() : theo_generator_(0.0) {}

    explicit BotContainer(std::string ticker, double starting_price) :
        TICKER(std::move(ticker)), theo_generator_(starting_price)
    {}
};
} // namespace bots
} // namespace nutc
