#include "orderbook.hpp"

#include "shared/util.hpp"

namespace nutc {
namespace matching {

void
OrderBook::clean_tree(util::Side side)
{
    auto& tree = side == util::Side::buy ? bids_ : asks_;
    while (!tree.empty()) {
        auto key = side == util::Side::buy ? std::prev(tree.end()) : tree.begin();
        auto& q = key->second;

        if (q.empty()) {
            tree.erase(key);
            continue;
        }

        if (q.front().was_removed) {
            q.pop();
            continue;
        }

        return;
    }
}

stored_order&
OrderBook::get_top_order(util::Side side)
{
    auto& tree = side == util::Side::buy ? bids_ : asks_;
    assert(!tree.empty());

    auto key = side == util::Side::buy ? std::prev(tree.end()) : tree.begin();
    auto& q = key->second;

    assert(!q.empty());
    return q.front();
}

void
OrderBook::modify_level_(util::Side side, decimal_price price, double delta)
{
    if (level_update_generator_)
        level_update_generator_->record_level_change(side, price, delta);
}

bool
OrderBook::can_match_orders()
{
    clean_tree(util::Side::sell);
    clean_tree(util::Side::buy);

    if (bids_.empty() || asks_.empty()) [[unlikely]] {
        return false;
    }
    return get_top_order(util::Side::buy).can_match(get_top_order(util::Side::sell));
}

decimal_price
OrderBook::get_midprice() const
{
    if (bids_.empty() || asks_.empty()) [[unlikely]] {
        return 0.0;
    }
    return (std::prev(bids_.end())->first + asks_.begin()->first) / 2;
}

std::vector<stored_order>
OrderBook::remove_ioc_orders()
{
    std::vector<stored_order> orders;
    for (uint64_t order_id : ioc_order_ids_) {
        orders.push_back(mark_order_removed(order_id));
    }

    ioc_order_ids_.clear();
    return orders;
}

void
OrderBook::change_quantity(stored_order& order, double quantity_delta)
{
    if (util::is_close_to_zero(order.quantity + quantity_delta)) {
        mark_order_removed(order);
        return;
    }

    order.trader.process_position_change(
        {order.side, order.ticker, order.price, quantity_delta, order.ioc}
    );

    order.quantity += quantity_delta;
    modify_level_(order.side, order.price, quantity_delta);
}

stored_order&
OrderBook::mark_order_removed(stored_order& order)
{
    order.trader.process_position_change(
        {order.side, order.ticker, order.price, -order.quantity, order.ioc}
    );

    modify_level_(order.side, order.price, -order.quantity);
    order.was_removed = true;

    order_map_.erase(order.order_index);
    return order;
}

stored_order&
OrderBook::mark_order_removed(uint64_t order_id)
{
    // TODO: remove once testing real algos
    assert(order_map_.contains(order_id));
    return mark_order_removed(order_map_.at(order_id));
}

void
OrderBook::add_order(stored_order order)
{
    order.trader.process_position_change(order);

    auto& map = order.side == util::Side::buy ? bids_ : asks_;
    map[order.price].push(order);

    order_map_.emplace(order.order_index, order);

    if (order.ioc) {
        ioc_order_ids_.push_back(order.order_index);
    }
    else {
        modify_level_(order.side, order.price, order.quantity);
    }
}

} // namespace matching
} // namespace nutc
