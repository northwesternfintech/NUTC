#pragma once

#include "common/types/decimal.hpp"
#include "common/util.hpp"

#include <vector>

namespace nutc::exchange {

class LevelQuantityTracker {
    static constexpr int START_SIZE = 256;
    std::vector<common::decimal_quantity> bid_levels_ =
        std::vector<common::decimal_quantity>(START_SIZE);
    std::vector<common::decimal_quantity> ask_levels_ =
        std::vector<common::decimal_quantity>(START_SIZE);

public:
    void report_quantity(
        common::Side side, common::decimal_quantity quantity_delta,
        common::decimal_price price
    );
    common::decimal_quantity
    get_level(common::Side side, common::decimal_price price) const;
};

} // namespace nutc::exchange
