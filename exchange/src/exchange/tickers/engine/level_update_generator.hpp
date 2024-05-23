#pragma once
#include "shared/messages_exchange_to_wrapper.hpp"

#include <unordered_set>

namespace nutc {
namespace matching {
using ob_update = messages::orderbook_update;

class OrderBook;

class LevelUpdateGenerator {
public:
    LevelUpdateGenerator() = default;

    void
    record_level_change(util::Side side, double price)
    {
        if (side == util::Side::buy) {
            updated_buy_levels_.insert(price);
        }
        else {
            updated_sell_levels_.insert(price);
        }
    }

    void
    erase_level_change(util::Side side, double price)
    {
        if (side == util::Side::buy) {
            updated_buy_levels_.erase(price);
        }
        else {
            updated_sell_levels_.erase(price);
        }
    }

    std::vector<ob_update>
    get_updates(const std::string& ticker, const OrderBook& orderbook);

    void
    reset()
    {
        updated_buy_levels_.clear();
        updated_sell_levels_.clear();
    }

private:
    std::unordered_set<double> updated_buy_levels_{};
    std::unordered_set<double> updated_sell_levels_{};
};
} // namespace matching
} // namespace nutc
