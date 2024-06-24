#include "level_update_generator.hpp"

#include <cassert>

namespace nutc {
namespace matching {
std::vector<ob_update>
LevelUpdateGenerator::get_updates(const std::string& ticker) const
{
    std::vector<ob_update> updates;
    updates.reserve(updated_buy_levels_.size() + updated_sell_levels_.size());

    for (uint32_t modified_decimal : modified_buy_levels_) {
        auto quantity = updated_buy_levels_[modified_decimal];
        updates.emplace_back(
            ticker, util::Side::buy, static_cast<double>(modified_decimal) / 100,
            quantity
        );
    }
    for (uint32_t modified_decimal : modified_sell_levels_) {
        auto quantity = updated_sell_levels_[modified_decimal];
        updates.emplace_back(
            ticker, util::Side::sell, static_cast<double>(modified_decimal) / 100,
            quantity
        );
    }

    return updates;
}

void
LevelUpdateGenerator::record_level_change(
    util::Side side, decimal_price price, double new_quantity
)
{
    auto& quantity_levels = side == util::Side::buy ? updated_buy_levels_ : updated_sell_levels_;
    auto& modified_levels = side == util::Side::buy ? modified_buy_levels_ : modified_sell_levels_;

    if (quantity_levels.size() <= price.price)
        quantity_levels.resize(static_cast<size_t>(price.price * 1.5));

    quantity_levels[price.price] = new_quantity;
	modified_levels.insert(price.price);
}
} // namespace matching
} // namespace nutc
