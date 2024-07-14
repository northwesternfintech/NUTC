#pragma once
#include "shared/types/decimal_price.hpp"
#include "exchange/orders/level_tracking/level_quantity_tracker.hpp"

#include "shared/types/position.hpp"

#include <unordered_set>

namespace nutc {
namespace matching {

class LevelUpdateGenerator {
	LevelQuantityTracker quantity_tracker_;

    std::unordered_set<util::decimal_price> modified_buy_levels_;
    std::unordered_set<util::decimal_price> modified_sell_levels_;

public:
    void record_level_change(util::Side side, util::decimal_price price, double delta);

    std::vector<util::position> get_updates(util::Ticker ticker) const;

    void
    reset()
    {
        modified_buy_levels_.clear();
        modified_sell_levels_.clear();
    }
};
} // namespace matching
} // namespace nutc
