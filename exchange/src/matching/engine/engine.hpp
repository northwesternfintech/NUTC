#pragma once

#include "client_manager/client_manager.hpp"
#include "logging.hpp"
#include "utils/logger/logger.hpp"
#include "utils/messages.hpp"

#include <chrono>

#include <optional>
#include <queue>
#include <vector>

using MarketOrder = nutc::messages::MarketOrder;
using ObUpdate = nutc::messages::ObUpdate;
using Match = nutc::messages::Match;
using SIDE = nutc::messages::SIDE;

namespace nutc {
/**
 * @brief Handles matching for an arbitrary ticker
 */
namespace matching {

struct MatchResult {
    std::vector<Match> matches;
    std::vector<ObUpdate> ob_updates;
};

class Engine {
public:
    std::priority_queue<MarketOrder> bids;
    std::priority_queue<MarketOrder> asks;

    /**
     * @brief Matches the given order against the current order book.
     * @param aggressive_order The order to match against the order book.
     * @param manager ClientManager to verify validity of orders/matches (correct
     * funds/holdings)
     * @return a MatchResult containing all matches and a vector containing the
     * orderbook updates
     */
    MatchResult
    match_order(MarketOrder& aggressive_order, manager::ClientManager& manager);

    void add_order_without_matching(MarketOrder aggressive_order);

private:
    float last_sell_price;
    static std::string get_client_id(
        SIDE side, const MarketOrder& aggressive, const MarketOrder& passive
    );
    float get_match_quantity(const MarketOrder& passive, const MarketOrder& aggressive);

    std::priority_queue<MarketOrder>& get_orders(SIDE side);

    MatchResult
    attempt_matches(manager::ClientManager& manager, const MarketOrder& aggressive);
    SIDE get_aggressive_side(const MarketOrder& order1, const MarketOrder& order2);
    bool insufficient_capital(
        const MarketOrder& order, const manager::ClientManager& manager
    );
};
} // namespace matching
} // namespace nutc
