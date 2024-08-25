#include "level_update_generator.hpp"

#include <cassert>

namespace nutc {
namespace matching {
std::vector<util::position>
LevelUpdateGenerator::get_updates(util::Ticker ticker) const
{
    std::vector<util::position> updates;

    for (util::decimal_price modified_decimal : modified_buy_levels_) {
        auto quantity = quantity_tracker_.get_level(util::Side::buy, modified_decimal);
        updates.emplace_back(ticker, util::Side::buy, quantity, modified_decimal);
    }
    for (util::decimal_price modified_decimal : modified_sell_levels_) {
        auto quantity = quantity_tracker_.get_level(util::Side::sell, modified_decimal);
        updates.emplace_back(ticker, util::Side::sell, quantity, modified_decimal);
    }

    return updates;
}

void
LevelUpdateGenerator::record_level_change(
    util::Side side, double quantity_delta, util::decimal_price price
)
{
    quantity_tracker_.report_quantity(side, quantity_delta, price);

    auto& modified_levels =
        side == util::Side::buy ? modified_buy_levels_ : modified_sell_levels_;

    modified_levels.insert(price);
}
} // namespace matching
} // namespace nutc
