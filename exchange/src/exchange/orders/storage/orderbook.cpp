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

std::optional<std::reference_wrapper<stored_order>>
OrderBook::get_top_order(util::Side side)
{
    clean_tree(side);

    auto& tree = side == util::Side::buy ? bids_ : asks_;

    if (tree.empty()) [[unlikely]]
        return std::nullopt;

    auto key = side == util::Side::buy ? std::prev(tree.end()) : tree.begin();
    auto& q = key->second;

    if (q.empty()) [[unlikely]]
        return std::nullopt;

    return q.front();
}

decimal_price
OrderBook::get_midprice() const
{
    if (bids_.empty() || asks_.empty()) [[unlikely]] {
        return 0.0;
    }
    return (std::prev(bids_.end())->first + asks_.begin()->first) / 2;
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
}

void
OrderBook::mark_order_removed(stored_order& order)
{
    order.trader.process_position_change(
        {order.side, order.ticker, order.price, -order.quantity, order.ioc}
    );

    order.was_removed = true;
}

stored_order&
OrderBook::add_order(stored_order order)
{
    order.trader.process_position_change(order);

    auto& map = order.side == util::Side::buy ? bids_ : asks_;

    auto& queue = map[order.price];
    queue.push(order);
    return queue.back();
}

} // namespace matching
} // namespace nutc
