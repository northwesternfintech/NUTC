#include "level_update_generator.hpp"

#include <cassert>

namespace nutc::exchange {

std::vector<shared::position>
LevelUpdateGenerator::get_updates(shared::Ticker ticker) const
{
    std::vector<shared::position> updates;

    for (shared::decimal_price modified_decimal : modified_buy_levels_) {
        auto quantity =
            quantity_tracker_.get_level(shared::Side::buy, modified_decimal);
        updates.emplace_back(ticker, shared::Side::buy, quantity, modified_decimal);
    }
    for (shared::decimal_price modified_decimal : modified_sell_levels_) {
        auto quantity =
            quantity_tracker_.get_level(shared::Side::sell, modified_decimal);
        updates.emplace_back(ticker, shared::Side::sell, quantity, modified_decimal);
    }

    return updates;
}

void
LevelUpdateGenerator::record_level_change(
    shared::Side side, double quantity_delta, shared::decimal_price price
)
{
    quantity_tracker_.report_quantity(side, quantity_delta, price);

    auto& modified_levels =
        side == shared::Side::buy ? modified_buy_levels_ : modified_sell_levels_;

    modified_levels.insert(price);
}
} // namespace nutc::exchange
