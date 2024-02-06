#pragma once
#include "exchange/bots/market_maker.hpp"
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

    size_t
    get_num_mm_bots() const
    {
        return market_makers_.size();
    }

    void add_mm_bot(float starting_capital);

    std::vector<MarketOrder> on_new_theo(float new_theo, float current);

    void process_bot_match(const Match& match);

    void process_order_expiration(
        const std::string& bot_id, messages::SIDE side, float total_cap
    );

    BotContainer() = default;

    explicit BotContainer(std::string ticker, double starting_price) :
        ticker_(std::move(ticker)), brownian_offset_(starting_price)
    {}

private:
    // TODO(stevenewald): make more elegant than string UUID
    std::unordered_map<std::string, MarketMakerBot> market_makers_{};
    std::string ticker_;

    stochastic::BrownianMotion theo_generator_{};
    double brownian_offset_ = 0.0;
};
} // namespace bots
} // namespace nutc
