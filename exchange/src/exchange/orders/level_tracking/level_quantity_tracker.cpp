#include "level_quantity_tracker.hpp"

namespace nutc {
namespace matching {
void
LevelQuantityTracker::report_quantity(
    util::Side side, double quantity_delta, util::decimal_price price
)
{
    if (price.price < 0) [[unlikely]]
        throw std::runtime_error("Reporting quantity less than 0");

    auto& levels = side == util::Side::buy ? bid_levels_ : ask_levels_;

    // TODO: guarantee size checks
    if (levels.size() <= static_cast<uint64_t>(price.price)) [[unlikely]] {
        auto new_size = static_cast<size_t>(static_cast<double>(price.price) * 1.5);
        bid_levels_.resize(new_size);
        ask_levels_.resize(new_size);
    }

    levels[static_cast<uint64_t>(price.price)] += quantity_delta;
}

double
LevelQuantityTracker::get_level(util::Side side, util::decimal_price price) const
{
    if (price.price < 0) [[unlikely]]
        throw std::runtime_error("Reporting quantity less than 0");

    const auto& levels = (side == util::Side::buy) ? bid_levels_ : ask_levels_;

    if (levels.size() <= static_cast<uint64_t>(price.price)) [[unlikely]] {
        return 0.0;
    }

    return levels[static_cast<uint64_t>(price.price)];
}
} // namespace matching
} // namespace nutc
