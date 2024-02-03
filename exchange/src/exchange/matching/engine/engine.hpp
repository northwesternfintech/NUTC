#pragma once

#include "exchange/client_manager/client_manager.hpp"
#include "order_storage.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <cassert>

#include <queue>
#include <set>
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

    std::vector<MarketOrder&> flag_orders_up_to_tick(uint64_t tick);

    void
    add_order(const MarketOrder& order)
    {
        return add_order(StoredOrder(
            order.client_id, order.side, order.ticker, order.quantity, order.price
        ));
    }

    void
    add_order(StoredOrder stored_order)
    {
        if (stored_order.side == SIDE::BUY) {
            bids_.insert(
                std::pair<float, uint64_t>(stored_order.price, stored_order.order_index)
            );
        }
        else {
            asks_.insert(
                std::pair<float, uint64_t>(stored_order.price, stored_order.order_index)
            );
        }

        orders_by_id_.insert({stored_order.order_index, std::move(stored_order)});
    }

private:
    // both map/sort price, order_index
    std::set<std::pair<float, uint64_t>, bid_comparator> bids_;
    std::set<std::pair<float, uint64_t>, ask_comparator> asks_;
    std::unordered_map<uint64_t, StoredOrder> orders_by_id_;

    std::map<uint64_t, std::queue<StoredOrder*>> orders_by_tick_;

    match_result_t
    attempt_matches_(manager::ClientManager& manager, StoredOrder& aggressive_order);

    StoredOrder&
    get_top_order_(SIDE side)
    {
        uint64_t order_id; // NOLINT(*)
        switch (side) {
            case SIDE::BUY:
                assert(!bids_.empty());
                order_id = bids_.begin()->second;
                while (orders_by_id_.find(order_id) == orders_by_id_.end()) {
                    bids_.erase(bids_.begin());
                    assert(!bids_.empty());
                    order_id = bids_.begin()->second;
                }
                return orders_by_id_.at(bids_.begin()->second);
            case SIDE::SELL:
                assert(!asks_.empty());
                order_id = asks_.begin()->second;
                while (orders_by_id_.find(order_id) == orders_by_id_.end()) {
                    asks_.erase(asks_.begin());
                    assert(!asks_.empty());
                    order_id = asks_.begin()->second;
                }
                return orders_by_id_.at(asks_.begin()->second);
        }
    }
};
} // namespace matching
} // namespace nutc
