#pragma once
#include "decimal_price.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

#include <unordered_set>

namespace nutc {
namespace matching {
using ob_update = messages::orderbook_update;

class LevelUpdateGenerator {
public:
    void record_level_change(util::Side side, decimal_price price, double new_quantity);

    std::vector<ob_update> get_updates(util::Ticker ticker) const;

    void
    reset()
    {
        modified_buy_levels_.clear();
        modified_sell_levels_.clear();
    }

private:
    std::vector<double> updated_buy_levels_{1000};
    std::vector<double> updated_sell_levels_{1000};

    std::unordered_set<uint32_t> modified_buy_levels_;
    std::unordered_set<uint32_t> modified_sell_levels_;
};
} // namespace matching
} // namespace nutc
