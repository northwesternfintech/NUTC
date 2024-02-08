#pragma once

#include "exchange/tick_manager/tick_observer.hpp"

#include <string>
#include <vector>

namespace nutc {
namespace dashboard {

// NOLINTBEGIN
struct TickerState : public ticks::TickObserver {
    const std::string TICKER;
    const float STARTING_PRICE;

    float theo_{};                     // done
    float midprice_{};                 // done
    std::pair<float, float> spread_{}; // done
    uint num_bids_{};                  // done
    uint num_asks_{};                  // done

    std::vector<int> mm_open_bids_{};
    std::vector<int> mm_open_asks_{};
    std::vector<float> mm_bid_interest_{};
    std::vector<float> mm_ask_interest_{};

    TickerState(std::string ticker, float starting_price) :
        TICKER(std::move(ticker)), STARTING_PRICE(starting_price)
    {}

    void on_tick(uint64_t tick) override;
};

} // namespace dashboard
} // namespace nutc

// NOLINTEND
