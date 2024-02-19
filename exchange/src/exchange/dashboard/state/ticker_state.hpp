#pragma once

#include <cmath>

#include <string>

namespace nutc {
namespace dashboard {

// NOLINTBEGIN
struct BotStates {
    size_t num_bots_{};
    size_t num_bots_active_{};
    double percent_active_{};

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

struct TickerState {
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

    void calculate_metrics();

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
            bot_states.num_bots_active_ = 0;
            bot_states.percent_active_ = 0;
            bot_states.min_open_bids_ = std::numeric_limits<size_t>::max();
            bot_states.min_open_asks_ = std::numeric_limits<size_t>::max();
            bot_states.max_open_bids_ = std::numeric_limits<size_t>::min();
            bot_states.max_open_asks_ = std::numeric_limits<size_t>::min();
            bot_states.min_utilization_ = std::numeric_limits<double>::max();
            bot_states.max_utilization_ = std::numeric_limits<double>::min();
            bot_states.min_pnl_ = std::numeric_limits<double>::max();
            bot_states.max_pnl_ = std::numeric_limits<double>::min();
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
