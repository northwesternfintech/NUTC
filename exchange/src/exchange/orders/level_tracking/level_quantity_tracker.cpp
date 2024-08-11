#include "level_quantity_tracker.hpp"

namespace nutc {
namespace matching {
void
LevelQuantityTracker::report_quantity(
    util::Side side, double quantity_delta, util::decimal_price price
)
{
    auto& levels = side == util::Side::buy ? bid_levels_ : ask_levels_;

    // TODO: guarantee size checks
    if (levels.size() <= price.price) [[unlikely]] {
        auto new_size = static_cast<size_t>(static_cast<double>(price.price) * 1.5);
        bid_levels_.resize(new_size);
        ask_levels_.resize(new_size);
    }

    levels[price.price] += quantity_delta;
}

double
LevelQuantityTracker::get_level(util::Side side, util::decimal_price price) const
{
    const auto& levels = (side == util::Side::buy) ? bid_levels_ : ask_levels_;

    if (levels.size() <= price.price) [[unlikely]] {
        return 0.0;
    }

    return levels[price.price];
}
} // namespace matching
} // namespace nutc
