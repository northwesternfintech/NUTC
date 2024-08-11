#pragma once

#include "shared/types/decimal_price.hpp"
#include "shared/util.hpp"

#include <vector>

namespace nutc {

namespace matching {
class LevelQuantityTracker {
    static constexpr auto START_SIZE = 256;
    std::vector<double> bid_levels_ = std::vector<double>(START_SIZE);
    std::vector<double> ask_levels_ = std::vector<double>(START_SIZE);

public:
    void
    report_quantity(util::Side side, double quantity_delta, util::decimal_price price);
    double get_level(util::Side side, util::decimal_price price) const;
};

} // namespace matching
} // namespace nutc
