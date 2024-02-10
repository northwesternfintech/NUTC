#pragma once

#include "exchange/tick_manager/tick_observer.hpp"

#include <cmath>

#include <string>
#include <vector>

namespace nutc {
namespace dashboard {

// NOLINTBEGIN
struct TickerState : public ticks::TickObserver {
    const std::string TICKER;
    const float STARTING_PRICE;

    float theo_{};
    float midprice_{};
    std::pair<float, float> spread_{};
    size_t num_bids_{};
    size_t num_asks_{};

    size_t num_mm_bots_{};

    size_t mm_min_open_bids_{};
    size_t mm_min_open_asks_{};
    size_t mm_max_open_bids_{};
    size_t mm_max_open_asks_{};
    float mm_min_utilization_{};
    float mm_max_utilization_{};

    size_t mm_avg_open_bids_{};
    size_t mm_avg_open_asks_{};
    float mm_avg_bid_interest_{};
    float mm_avg_ask_interest_{};
    float mm_avg_utilization_{};

    // std::vector<size_t> mm_open_bids_{};
    // std::vector<int> mm_open_asks_{};
    // std::vector<float> mm_bid_interest_{};
    // std::vector<float> mm_ask_interest_{};
    // std::vector<float> mm_utilization_{};

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

        num_mm_bots_ = 0;
        mm_min_open_bids_ = INFINITY;
        mm_min_open_asks_ = INFINITY;
        mm_max_open_bids_ = -INFINITY;
        mm_max_open_asks_ = -INFINITY;
        mm_min_utilization_ = static_cast<float>(INFINITY);
        mm_max_utilization_ = -static_cast<float>(INFINITY);
        mm_avg_open_bids_ = 0;
        mm_avg_open_asks_ = 0;
        mm_avg_bid_interest_ = 0;
        mm_avg_ask_interest_ = 0;
        mm_avg_utilization_ = 0;
        // mm_open_bids_.clear();
        // mm_open_asks_.clear();
        // mm_bid_interest_.clear();
        // mm_ask_interest_.clear();
        // mm_utilization_.clear();
    }
};

} // namespace dashboard
} // namespace nutc

// NOLINTEND
