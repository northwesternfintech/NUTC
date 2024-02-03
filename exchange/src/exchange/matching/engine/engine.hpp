#pragma once

#include "exchange/client_manager/client_manager.hpp"
#include "order_storage.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

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

struct match_result_t {
    std::vector<Match> matches;
    std::vector<ObUpdate> ob_updates;
};

class Engine {
public:
    /**
     * @brief Matches the given order against the current order book.
     * @param aggressive_order The order to match against the order book.
     * @param manager ClientManager to verify validity of orders/matches (correct
     * funds/holdings)
     * @return a MatchResult containing all matches and a vector containing the
     * orderbook updates
     */
    match_result_t match_order(MarketOrder&& order, manager::ClientManager& manager);

    void add_order_without_matching(const MarketOrder& order);

private:
    std::priority_queue<StoredOrder> bids_;
    std::priority_queue<StoredOrder> asks_;
    std::priority_queue<StoredOrder>& get_orders_(SIDE side);

    match_result_t attempt_matches_(
        manager::ClientManager& manager, const StoredOrder& aggressive_order
    );
};
} // namespace matching
} // namespace nutc
