#include "level_update_generator.hpp"

#include <unordered_set>

namespace nutc {
namespace matching {

// There is a more efficient way to do this, but it's O(levels) which will be <=100
// in most cases and only called on tick
// so simplicity > performance
std::vector<ob_update>
get_updates(const std::string& ticker, const OrderBook& before, const OrderBook& after)
{
    std::unordered_set<double> buy_levels{};
    std::unordered_set<double> ask_levels{};
    for (auto [price, _] : before.get_levels(util::Side::buy)) {
        buy_levels.insert(price);
    }
    for (auto [price, _] : before.get_levels(util::Side::sell)) {
        ask_levels.insert(price);
    }
    for (auto [price, _] : after.get_levels(util::Side::buy)) {
        buy_levels.insert(price);
    }
    for (auto [price, _] : after.get_levels(util::Side::sell)) {
        ask_levels.insert(price);
    }

    std::vector<ob_update> updates{};
    for (auto price : buy_levels) {
        double before_quantity = before.get_level(util::Side::buy, price);
        double after_quantity = after.get_level(util::Side::buy, price);

        if (before_quantity == after_quantity)
            continue;

        // Round to 3 places
        before_quantity = std::round(before_quantity * 1000) / 1000;
        after_quantity = std::round(after_quantity * 1000) / 1000;

        updates.emplace_back(ticker, util::Side::buy, price, after_quantity);
    }

    for (auto price : ask_levels) {
        double before_quantity = before.get_level(util::Side::sell, price);
        double after_quantity = after.get_level(util::Side::sell, price);

        if (before_quantity == after_quantity)
            continue;

        updates.emplace_back(ticker, util::Side::sell, price, after_quantity);
    }

    return updates;
}
} // namespace matching
} // namespace nutc
