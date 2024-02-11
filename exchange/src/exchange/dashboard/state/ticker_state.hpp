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
    size_t num_retail_bots_{};

    // MM stats
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

    // Retail stats
    size_t retail_min_open_bids_{};
    size_t retail_min_open_asks_{};
    size_t retail_max_open_bids_{};
    size_t retail_max_open_asks_{};
    float retail_min_utilization_{};
    float retail_max_utilization_{};
    float retail_min_pnl_{};
    float retail_max_pnl_{};

    size_t retail_avg_open_bids_{};
    size_t retail_avg_open_asks_{};
    float retail_avg_bid_interest_{};
    float retail_avg_ask_interest_{};
    float retail_avg_utilization_{};
    float retail_avg_pnl_{};

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

        num_retail_bots_ = 0;
        retail_min_open_bids_ = INFINITY;
        retail_min_open_asks_ = INFINITY;
        retail_max_open_bids_ = -INFINITY;
        retail_max_open_asks_ = -INFINITY;
        retail_min_utilization_ = static_cast<float>(INFINITY);
        retail_max_utilization_ = -static_cast<float>(INFINITY);
        retail_min_pnl_ = static_cast<float>(INFINITY);
        retail_max_pnl_ = -static_cast<float>(INFINITY);
        retail_avg_open_bids_ = 0;
        retail_avg_open_asks_ = 0;
        retail_avg_bid_interest_ = 0;
        retail_avg_ask_interest_ = 0;
        retail_avg_utilization_ = 0;
        retail_avg_pnl_ = 0;
    }
};

} // namespace dashboard
} // namespace nutc

// NOLINTEND
