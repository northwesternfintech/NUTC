#pragma once
#include "common/types/decimal.hpp"
#include "common/types/position.hpp"
#include "exchange/orders/level_tracking/level_quantity_tracker.hpp"

#include <unordered_set>

namespace nutc::exchange {

class LevelUpdateGenerator {
    common::Ticker ticker_;
    LevelQuantityTracker quantity_tracker_;

    std::unordered_set<common::decimal_price> modified_buy_levels_;
    std::unordered_set<common::decimal_price> modified_sell_levels_;

public:
    LevelUpdateGenerator() = delete;

    explicit LevelUpdateGenerator(common::Ticker ticker) : ticker_{ticker} {}

    void record_level_change(
        common::Side side, common::decimal_quantity quantity_delta,
        common::decimal_price price
    );

    std::vector<common::position> get_updates() const;

    void
    reset()
    {
        modified_buy_levels_.clear();
        modified_sell_levels_.clear();
    }
};
} // namespace nutc::exchange
