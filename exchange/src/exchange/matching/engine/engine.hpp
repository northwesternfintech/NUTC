#pragma once

#include "exchange/client_manager/client_manager.hpp"
#include "shared/messages.hpp"

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
    match_result_t match_order(MarketOrder& order, manager::ClientManager& manager);

    void add_order_without_matching(const MarketOrder& order);

private:
    std::priority_queue<MarketOrder> bids_;
    std::priority_queue<MarketOrder> asks_;
    static std::string
    get_client_id(SIDE side, const MarketOrder& aggressive, const MarketOrder& passive);
    static float get_match_quantity(
        const MarketOrder& passive_order, const MarketOrder& aggressive_order
    );

    std::priority_queue<MarketOrder>& get_orders_(SIDE side);

    match_result_t attempt_matches_(
        manager::ClientManager& manager, const MarketOrder& aggressive_order
    );
    static SIDE
    get_aggressive_side(const MarketOrder& order1, const MarketOrder& order2);
    static bool insufficient_capital(
        const MarketOrder& order, const manager::ClientManager& manager
    );
};
} // namespace matching
} // namespace nutc
