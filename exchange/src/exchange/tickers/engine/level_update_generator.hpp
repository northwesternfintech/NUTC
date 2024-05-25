#pragma once
#include "shared/messages_exchange_to_wrapper.hpp"

#include <unordered_map>

namespace nutc {
namespace matching {
using ob_update = messages::orderbook_update;

class LevelUpdateGenerator {
public:
    void record_level_change(util::Side side, double price, double new_quantity);

    std::vector<ob_update> get_updates(const std::string& ticker) const;

    void
    reset()
    {
        updated_buy_levels_.clear();
        updated_sell_levels_.clear();
    }

private:
    std::unordered_map<double, double> updated_buy_levels_{};
    std::unordered_map<double, double> updated_sell_levels_{};
};
} // namespace matching
} // namespace nutc
