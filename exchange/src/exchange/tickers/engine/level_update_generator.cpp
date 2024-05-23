#include "level_update_generator.hpp"
#include "order_container.hpp"

namespace nutc {
namespace matching {
    std::vector<ob_update> LevelUpdateGenerator::get_updates(const std::string& ticker, const OrderBook& orderbook)
    {
      std::vector<ob_update> updates;
      updates.reserve(updated_buy_levels_.size() + updated_sell_levels_.size());
      for (auto price : updated_buy_levels_) {
          double quantity = orderbook.get_level(util::Side::buy, price);
          updates.emplace_back(ticker, util::Side::buy, price, quantity);
      }
      for (auto price : updated_sell_levels_) {
          double quantity = orderbook.get_level(util::Side::sell, price);
          updates.emplace_back(ticker, util::Side::sell, price, quantity);
      }
      return updates;
    }
} // namespace matching
} // namespace nutc
