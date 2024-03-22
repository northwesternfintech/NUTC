#include "level_update_generator.hpp"

#include <unordered_set>

namespace nutc {
namespace matching {

// There is a more efficient way to do this, but it's O(levels) which will be <=100
// in most cases and only called on tick
// so simplicity > performance
std::vector<ObUpdate>
get_updates(
    const std::string& ticker, const OrderContainer& before, const OrderContainer& after
)
{
    std::unordered_set<double> buy_levels;
    std::unordered_set<double> ask_levels;
    for (auto [price, quantity] : before.get_levels(SIDE::BUY)) {
        buy_levels.insert(price);
    }
    for (auto [price, quantity] : before.get_levels(SIDE::SELL)) {
        ask_levels.insert(price);
    }
    for (auto [price, quantity] : after.get_levels(SIDE::BUY)) {
        buy_levels.insert(price);
    }
    for (auto [price, quantity] : after.get_levels(SIDE::SELL)) {
        ask_levels.insert(price);
    }

    std::vector<ObUpdate> updates;
    for (auto price : buy_levels) {
        double before_quantity = before.get_level(SIDE::BUY, price);
        double after_quantity = after.get_level(SIDE::BUY, price);

        if (before_quantity == after_quantity)
            continue;

        updates.push_back(ObUpdate{ticker, SIDE::BUY, price, after_quantity});
    }

    for (auto price : ask_levels) {
        double before_quantity = before.get_level(SIDE::SELL, price);
        double after_quantity = after.get_level(SIDE::SELL, price);

        if (before_quantity == after_quantity)
            continue;

        updates.push_back(ObUpdate{ticker, SIDE::SELL, price, after_quantity});
    }

    return updates;
}
} // namespace matching
} // namespace nutc
