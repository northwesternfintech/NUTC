#pragma once

#include "exchange/tick_manager/tick_observer.hpp"

#include <cmath>

#include <string>
#include <vector>

namespace nutc {
namespace dashboard {

// NOLINTBEGIN
struct BotStates {
    size_t num_bots_{};
    size_t min_open_bids_{};
    size_t min_open_asks_{};
    size_t max_open_bids_{};
    size_t max_open_asks_{};
    float min_utilization_{};
    float max_utilization_{};
    float min_pnl_{};
    float max_pnl_{};

    size_t avg_open_bids_{};
    size_t avg_open_asks_{};
    float avg_bid_interest_{};
    float avg_ask_interest_{};
    float avg_utilization_{};
    float avg_pnl_{};
};

struct TickerState : public ticks::TickObserver {
    const std::string TICKER;
    const float STARTING_PRICE;

    float theo_{};
    float midprice_{};
    std::pair<float, float> spread_{};
    size_t num_bids_{};
    size_t num_asks_{};

    BotStates mm_state_{};
    BotStates retail_state_{};

    TickerState(std::string ticker, float starting_price) :
        TICKER(std::move(ticker)), STARTING_PRICE(starting_price)
    {}

    void on_tick(uint64_t tick) override;

private:
    void
    reset_()
    {
        theo_ = 0;
        midprice_ = 0;
        spread_ = {0, 0};
        num_bids_ = 0;
        num_asks_ = 0;

        auto reset_bot_states = [](BotStates& bot_states) {
            bot_states.num_bots_ = 0;
            bot_states.min_open_bids_ = INFINITY;
            bot_states.min_open_asks_ = INFINITY;
            bot_states.max_open_bids_ = -INFINITY;
            bot_states.max_open_asks_ = -INFINITY;
            bot_states.min_utilization_ = static_cast<float>(INFINITY);
            bot_states.max_utilization_ = -static_cast<float>(INFINITY);
            bot_states.min_pnl_ = static_cast<float>(INFINITY);
            bot_states.max_pnl_ = -static_cast<float>(INFINITY);
            bot_states.avg_open_bids_ = 0;
            bot_states.avg_open_asks_ = 0;
            bot_states.avg_bid_interest_ = 0;
            bot_states.avg_ask_interest_ = 0;
            bot_states.avg_utilization_ = 0;
            bot_states.avg_pnl_ = 0;
        };

        reset_bot_states(mm_state_);
        reset_bot_states(retail_state_);
    }
};

} // namespace dashboard
} // namespace nutc

// NOLINTEND
