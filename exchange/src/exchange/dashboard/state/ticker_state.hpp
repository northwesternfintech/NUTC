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
    double min_utilization_{};
    double max_utilization_{};
    double min_pnl_{};
    double max_pnl_{};

    double avg_open_bids_{};
    double avg_open_asks_{};
    double avg_bid_interest_{};
    double avg_ask_interest_{};
    double avg_utilization_{};
    double avg_pnl_{};
};

struct TickerState : public ticks::TickObserver {
    const std::string TICKER;
    const double STARTING_PRICE;

    double theo_{};
    double midprice_{};
    std::pair<double, double> spread_{};
    size_t num_bids_{};
    size_t num_asks_{};

    BotStates mm_state_{};
    BotStates retail_state_{};

    TickerState(std::string ticker, double starting_price) :
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
            bot_states.min_utilization_ = static_cast<double>(INFINITY);
            bot_states.max_utilization_ = -static_cast<double>(INFINITY);
            bot_states.min_pnl_ = static_cast<double>(INFINITY);
            bot_states.max_pnl_ = -static_cast<double>(INFINITY);
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
