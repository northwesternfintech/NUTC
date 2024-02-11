#pragma once

#include "exchange/traders/trader_manager.hpp"
#include "order_storage.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <functional>
#include <queue>
#include <set>
#include <vector>

using MarketOrder = nutc::messages::MarketOrder;
using ObUpdate = nutc::messages::ObUpdate;
using Match = nutc::messages::Match;
using SIDE = nutc::messages::SIDE;

// this class is very messy and should be refactored at some point
// there's minimal abstraction to support high performance, but this makes the code look
// like garbage

namespace nutc {
/**
 * @brief Handles matching for an arbitrary ticker
 */
namespace matching {

struct match_result_t {
    std::vector<Match> matches;
    std::vector<ObUpdate> ob_updates;
};

struct on_tick_result_t {
    std::vector<StoredOrder> removed_orders;
    std::vector<StoredOrder> added_orders;
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

    std::pair<uint, uint>
    get_spread_nums() const
    {
        return {asks_.size(), bids_.size()};
    }

    std::pair<float, float>
    get_spread() const
    {
        if (asks_.empty() || bids_.empty()) [[unlikely]] {
            return {0, 0};
        }
        return {asks_.begin()->price, bids_.rbegin()->price};
    }

    float
    get_midprice() const
    {
        if (asks_.empty() || bids_.empty()) [[unlikely]] {
            return initial_price_;
        }
        return (asks_.begin()->price + bids_.rbegin()->price) / 2;
    }

    void
    add_order(const MarketOrder& order)
    {
        return add_order(StoredOrder(
            manager::ClientManager::get_instance().get_generic_trader(order.client_id),
            order.side, order.ticker, order.quantity, order.price, this->current_tick_
        ));
    }

    void
    set_initial_price(float price)
    {
        initial_price_ = price;
    }

    void
    add_order(const StoredOrder& stored_order)
    {
        switch (stored_order.side) {
            case SIDE::BUY:
                bids_.insert({stored_order.price, stored_order.order_index});
                break;
            case SIDE::SELL:
                asks_.insert({stored_order.price, stored_order.order_index});
        }

        orders_by_id_.emplace(stored_order.order_index, stored_order);
        orders_by_tick_[stored_order.tick].push(stored_order.order_index);
    }

    // Called every tick
    on_tick_result_t on_tick(uint64_t new_tick, uint8_t order_expire_age);

private:
    uint64_t current_tick_ = 0;
    float initial_price_;

    match_result_t
    attempt_matches_(manager::ClientManager& manager, StoredOrder& aggressive_order);

    // both map/sort price, order_index
    std::set<order_index, bid_comparator> bids_;
    std::set<order_index, ask_comparator> asks_;

    // order index -> order
    std::unordered_map<uint64_t, StoredOrder> orders_by_id_;

    // tick -> queue of order ids
    std::map<uint64_t, std::queue<uint64_t>> orders_by_tick_;

    std::vector<StoredOrder> removed_orders_{};
    std::vector<StoredOrder> added_orders_{};

    template <typename Comparator>
    std::optional<std::reference_wrapper<StoredOrder>>
    get_order_from_set_(std::set<order_index, Comparator>& order_set)
    {
        if (order_set.empty()) {
            return std::nullopt;
        }

        auto order_id = order_set.begin()->index;
        while (orders_by_id_.find(order_id) == orders_by_id_.end()) {
            order_set.erase(order_set.begin());
            if (order_set.empty()) {
                return std::nullopt;
            }
            order_id = order_set.begin()->index;
        }
        return orders_by_id_.at(order_id);
    }

    std::optional<std::reference_wrapper<StoredOrder>>
    get_top_order_(SIDE side)
    {
        switch (side) {
            case SIDE::BUY:
                return get_order_from_set_(bids_);
            case SIDE::SELL:
                return get_order_from_set_(asks_);
            default:
                throw std::invalid_argument("Unknown side");
        }
    }

    bool
    can_match_orders_()
    {
        auto bid_order = get_top_order_(SIDE::BUY);
        auto ask_order = get_top_order_(SIDE::SELL);
        if (!bid_order.has_value() || !ask_order.has_value()) {
            return false;
        }
        return bid_order.value().get().can_match(ask_order.value().get());
    }
};
} // namespace matching
} // namespace nutc