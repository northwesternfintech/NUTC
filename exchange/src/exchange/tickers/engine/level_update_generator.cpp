#include "level_update_generator.hpp"

#include <algorithm>
#include <ranges>

namespace nutc {
namespace matching {
std::vector<ob_update>
LevelUpdateGenerator::get_updates(const std::string& ticker)
{
    auto level_to_update = [&](auto& level) -> ob_update {
        return {ticker, util::Side::buy, level.first, level.second};
    };

    std::vector<ob_update> updates;
    updates.reserve(updated_buy_levels_.size() + updated_sell_levels_.size());

    // std::transform(
    //     updated_buy_levels_.begin(), updated_buy_levels_.end(),
    //     std::back_inserter(updates), level_to_update
    // );
    for (auto [price, quantity] : updated_buy_levels_) {
        updates.emplace_back(ticker, util::Side::buy, price, quantity);
    }
    for (auto [price, quantity] : updated_sell_levels_) {
        updates.emplace_back(ticker, util::Side::sell, price, quantity);
    }
    return updates;
}

void
LevelUpdateGenerator::record_level_change(
    util::Side side, double price, double new_quantity
)
{
    assert(std::round(price * 100) / 100 == price);
    if (side == util::Side::buy) {
        updated_buy_levels_[price] = new_quantity;
    }
    else {
        updated_sell_levels_[price] = new_quantity;
    }
}
} // namespace matching
} // namespace nutc
