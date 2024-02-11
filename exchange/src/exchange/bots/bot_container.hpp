#pragma once
#include "exchange/bots/bot_types/market_maker.hpp"
#include "exchange/bots/bot_types/retail.hpp"
#include "exchange/theo/brownian.hpp"
#include "exchange/tick_manager/tick_observer.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

namespace nutc {

namespace bots {
using MarketOrder = messages::MarketOrder;
using Match = messages::Match;

class BotContainer : public ticks::TickObserver {
public:
    void on_tick(uint64_t) override;

    double
    get_theo() const
    {
        return brownian_offset_ + theo_generator_.get_price();
    }

    const std::unordered_map<std::string, MarketMakerBot>&
    get_market_makers() const
    {
        return market_makers_;
    }

    const std::unordered_map<std::string, RetailBot>&
    get_retail_traders() const
    {
        return retail_bots_;
    }

    std::vector<MarketOrder> on_new_theo(double new_theo, double current);

    void process_order_expiration(
        const std::string& bot_id, messages::SIDE side, double total_cap
    );
    void
    process_order_add(const std::string& bot_id, messages::SIDE side, double total_cap);
    void process_order_match(Match& match);

    void add_mm_bots(const std::vector<double>& starting_capitals);
    void add_retail_bots(double mean_capital, double stddev_capital, int num_bots);

    BotContainer() = default;

    explicit BotContainer(std::string ticker, double starting_price) :
        ticker_(std::move(ticker)), brownian_offset_(starting_price)
    {}

private:
    void add_mm_bot_(double starting_capital);
    void add_retail_bot_(double starting_capital);
    std::unordered_map<std::string, RetailBot> retail_bots_{};
    std::unordered_map<std::string, MarketMakerBot> market_makers_{};
    std::string ticker_;

    stochastic::BrownianMotion theo_generator_{};
    double brownian_offset_ = 0.0;
};
} // namespace bots
} // namespace nutc
