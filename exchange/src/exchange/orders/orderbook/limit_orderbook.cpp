#include "limit_orderbook.hpp"

#include "shared/types/decimal_price.hpp"
#include "shared/util.hpp"

namespace nutc {
namespace matching {

void
LimitOrderBook::clean_tree(util::Side side)
{
    auto& tree = side == util::Side::buy ? bids_ : asks_;
    while (!tree.empty()) {
        auto key = side == util::Side::buy ? std::prev(tree.end()) : tree.begin();
        auto& q = key->second;

        if (q.empty()) {
            tree.erase(key);
            continue;
        }

        if (!q.front().active) {
            q.pop();
            continue;
        }

        return;
    }
}

auto
LimitOrderBook::get_top_order(util::Side side)
    -> std::optional<std::reference_wrapper<OrderT>>
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

util::decimal_price
LimitOrderBook::get_midprice() const
{
    if (bids_.empty() || asks_.empty()) [[unlikely]] {
        return 0.0;
    }
    return (std::prev(bids_.end())->first + asks_.begin()->first) / 2.0;
}

void
LimitOrderBook::change_quantity(OrderT& order, double quantity_delta)
{
    if (util::is_close_to_zero(order.quantity + quantity_delta)) {
        mark_order_removed(order);
        return;
    }

    order.trader->notify_position_change(
        {order.ticker, order.side, quantity_delta, order.price}
    );

    order.quantity += quantity_delta;
}

void
LimitOrderBook::mark_order_removed(OrderT& order)
{
    order.trader->notify_position_change(
        {order.ticker, order.side, -order.quantity, order.price}
    );

    order.active = false;
}

auto
LimitOrderBook::add_order(const OrderT& order) -> OrderT&
{
    order.trader->notify_position_change(
        {order.ticker, order.side, order.quantity, order.price}
    );

    auto& map = order.side == util::Side::buy ? bids_ : asks_;

    auto& queue = map[order.price];
    queue.push(order);
    return queue.back();
}

} // namespace matching
} // namespace nutc
