#pragma once

#include "shared/types/decimal.hpp"
#include "shared/util.hpp"

#include <vector>

namespace nutc::exchange {

class LevelQuantityTracker {
    static constexpr auto START_SIZE = 256;
    std::vector<double> bid_levels_ = std::vector<double>(START_SIZE);
    std::vector<double> ask_levels_ = std::vector<double>(START_SIZE);

public:
    void report_quantity(
        shared::Side side, double quantity_delta, shared::decimal_price price
    );
    double get_level(shared::Side side, shared::decimal_price price) const;
};

} // namespace nutc::exchange
