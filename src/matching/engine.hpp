#pragma once

#include "client_manager/manager.hpp"
#include "logging.hpp"
#include "util/messages.hpp"

#include <chrono>

#include <optional>
#include <queue>
#include <vector>

using MarketOrder = nutc::messages::MarketOrder;
using ObUpdate = nutc::messages::ObUpdate;
using Match = nutc::messages::Match;

namespace nutc {
/**
 * @brief Handles matching for an arbitrary ticker
 */
namespace matching {

class Engine {
public:
    std::priority_queue<MarketOrder> bids;
    std::priority_queue<MarketOrder> asks;

    /**
     * @brief Matches the given order against the current order book.
     * @param aggressive_order The order to match against the order book.
     * @param manager ClientManager to verify validity of orders/matches (correct
     * funds/holdings)
     * @return A pair of vectors, the first containing all matches, the second
     * containing the orderbook updates
     */
    std::pair<std::vector<Match>, std::vector<ObUpdate>>
    match_order(MarketOrder aggressive_order, const manager::ClientManager& manager);

    static ObUpdate create_ob_update(const MarketOrder& order, float quantity);
    Engine(); // con

private:
    void add_order(MarketOrder aggressive_order);
    std::pair<std::vector<Match>, std::vector<ObUpdate>> attempt_matches(
        std::priority_queue<MarketOrder>& passive_orders, MarketOrder& aggressive_order,
        const manager::ClientManager& manager
    );
};
} // namespace matching
} // namespace nutc
