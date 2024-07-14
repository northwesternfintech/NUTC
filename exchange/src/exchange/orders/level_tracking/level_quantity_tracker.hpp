#pragma once

#include "shared/types/decimal_price.hpp"
#include "shared/util.hpp"

#include <vector>

namespace nutc {

namespace matching {
class LevelQuantityTracker {
    std::vector<double> bid_levels_{100};
    std::vector<double> ask_levels_{100};

public:
    void report_quantity(util::Side side, util::decimal_price price, double delta);
    double get_level(util::Side side, util::decimal_price price) const;
};

} // namespace matching
} // namespace nutc
