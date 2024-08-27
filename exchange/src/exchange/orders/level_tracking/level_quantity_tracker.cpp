#include "level_quantity_tracker.hpp"

#include "common/types/decimal.hpp"

namespace nutc::exchange {

void
LevelQuantityTracker::report_quantity(
    common::Side side, common::decimal_quantity quantity_delta,
    common::decimal_price price
)
{
    if (price.get_underlying() < 0) [[unlikely]]
        throw std::runtime_error("Reporting quantity less than 0");

    auto& levels = side == common::Side::buy ? bid_levels_ : ask_levels_;

    // TODO: guarantee size checks
    if (levels.size() <= static_cast<uint64_t>(price.get_underlying())) [[unlikely]] {
        auto new_size =
            static_cast<size_t>(static_cast<double>(price.get_underlying()) * 1.5);
        bid_levels_.resize(new_size);
        ask_levels_.resize(new_size);
    }

    levels[static_cast<uint64_t>(price.get_underlying())] += quantity_delta;
}

common::decimal_quantity
LevelQuantityTracker::get_level(common::Side side, common::decimal_price price) const
{
    if (price.get_underlying() < 0) [[unlikely]]
        throw std::runtime_error("Reporting quantity less than 0");

    const auto& levels = (side == common::Side::buy) ? bid_levels_ : ask_levels_;

    if (levels.size() <= static_cast<uint64_t>(price.get_underlying())) [[unlikely]] {
        return 0.0;
    }

    return levels[static_cast<uint64_t>(price.get_underlying())];
}
} // namespace nutc::exchange
