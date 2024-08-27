#pragma once

#include "shared/types/decimal.hpp"
#include "shared/util.hpp"

#include <vector>

namespace nutc::exchange {

class LevelQuantityTracker {
    static constexpr int START_SIZE = 256;
    std::vector<shared::decimal_quantity> bid_levels_ =
        std::vector<shared::decimal_quantity>(START_SIZE);
    std::vector<shared::decimal_quantity> ask_levels_ =
        std::vector<shared::decimal_quantity>(START_SIZE);

public:
    void report_quantity(
        shared::Side side, shared::decimal_quantity quantity_delta,
        shared::decimal_price price
    );
    shared::decimal_quantity
    get_level(shared::Side side, shared::decimal_price price) const;
};

} // namespace nutc::exchange
