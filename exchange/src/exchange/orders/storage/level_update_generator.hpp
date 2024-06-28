#pragma once
#include "exchange/orders/storage/decimal_price.hpp"
#include "exchange/orders/storage/level_quantity_tracker.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

#include <unordered_set>

namespace nutc {
namespace matching {
using ob_update = messages::orderbook_update;

class LevelUpdateGenerator {
	LevelQuantityTracker quantity_tracker_;

    std::unordered_set<decimal_price> modified_buy_levels_;
    std::unordered_set<decimal_price> modified_sell_levels_;

public:
    void record_level_change(util::Side side, decimal_price price, double delta);

    std::vector<ob_update> get_updates(util::Ticker ticker) const;

    void
    reset()
    {
        modified_buy_levels_.clear();
        modified_sell_levels_.clear();
    }
};
} // namespace matching
} // namespace nutc
