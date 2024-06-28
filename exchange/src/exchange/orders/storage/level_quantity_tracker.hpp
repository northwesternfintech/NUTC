#pragma once

#include "exchange/orders/storage/decimal_price.hpp"
#include "shared/util.hpp"

#include <vector>

namespace nutc {

namespace matching {

class LevelQuantityTracker {
    std::vector<double> bid_levels_{1000};
    std::vector<double> ask_levels_{1000};

public:
    void
    report_quantity(util::Side side, decimal_price price, double delta)
    {
        auto& levels = side == util::Side::buy ? bid_levels_ : ask_levels_;

        if (levels.size() <= price.price) [[unlikely]] {
            bid_levels_.resize(static_cast<size_t>(price.price * 1.5));
            ask_levels_.resize(static_cast<size_t>(price.price * 1.5));
        }

        levels[price.price] += delta;
    }

    double
    get_level(util::Side side, decimal_price price) const
    {
        const auto& levels = (side == util::Side::buy) ? bid_levels_ : ask_levels_;

        if (levels.size() <= price.price) [[unlikely]] {
            return 0.0;
        }

        return levels[price.price];
    }
};

} // namespace matching
} // namespace nutc
