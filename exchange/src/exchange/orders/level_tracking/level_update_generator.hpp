#pragma once
#include "exchange/orders/level_tracking/level_quantity_tracker.hpp"
#include "shared/types/decimal.hpp"
#include "shared/types/position.hpp"

#include <unordered_set>

namespace nutc::exchange {

class LevelUpdateGenerator {
    shared::Ticker ticker_;
    LevelQuantityTracker quantity_tracker_;

    std::unordered_set<shared::decimal_price> modified_buy_levels_;
    std::unordered_set<shared::decimal_price> modified_sell_levels_;

public:
    LevelUpdateGenerator() = delete;

    explicit LevelUpdateGenerator(shared::Ticker ticker) : ticker_{ticker} {}

    void record_level_change(
        shared::Side side, double quantity_delta, shared::decimal_price price
    );

    std::vector<shared::position> get_updates() const;

    void
    reset()
    {
        modified_buy_levels_.clear();
        modified_sell_levels_.clear();
    }
};
} // namespace nutc::exchange
