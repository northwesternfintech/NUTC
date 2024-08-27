#include "level_update_generator.hpp"

#include <cassert>

namespace nutc::exchange {

std::vector<common::position>
LevelUpdateGenerator::get_updates() const
{
    std::vector<common::position> updates;

    for (common::decimal_price modified_decimal : modified_buy_levels_) {
        auto quantity =
            quantity_tracker_.get_level(common::Side::buy, modified_decimal);
        updates.emplace_back(ticker_, common::Side::buy, quantity, modified_decimal);
    }
    for (common::decimal_price modified_decimal : modified_sell_levels_) {
        auto quantity =
            quantity_tracker_.get_level(common::Side::sell, modified_decimal);
        updates.emplace_back(ticker_, common::Side::sell, quantity, modified_decimal);
    }

    return updates;
}

void
LevelUpdateGenerator::record_level_change(
    common::Side side, common::decimal_quantity quantity_delta,
    common::decimal_price price
)
{
    quantity_tracker_.report_quantity(side, quantity_delta, price);

    auto& modified_levels =
        side == common::Side::buy ? modified_buy_levels_ : modified_sell_levels_;

    modified_levels.insert(price);
}
} // namespace nutc::exchange
