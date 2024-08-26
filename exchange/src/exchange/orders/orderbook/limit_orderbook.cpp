#include "limit_orderbook.hpp"

#include "exchange/orders/storage/order_storage.hpp"
#include "shared/types/decimal_price.hpp"
#include "shared/util.hpp"

namespace nutc::exchange {

void
LimitOrderBook::clean_tree(shared::Side side)
{
    auto& tree = side == shared::Side::buy ? bids_ : asks_;
    while (!tree.empty()) {
        auto key = side == shared::Side::buy ? std::prev(tree.end()) : tree.begin();
        auto& queue = key->second;

        if (queue.empty()) {
            tree.erase(key);
            continue;
        }

        return;
    }
}

std::optional<LimitOrderBook::order_list::iterator>
LimitOrderBook::get_top_order(shared::Side side)
{
    clean_tree(side);

    auto& tree = side == shared::Side::buy ? bids_ : asks_;

    if (tree.empty()) [[unlikely]]
        return std::nullopt;

    auto key = side == shared::Side::buy ? std::prev(tree.end()) : tree.begin();
    auto& queue = key->second;

    if (queue.empty()) [[unlikely]]
        return std::nullopt;

    return queue.begin();
}

shared::decimal_price
LimitOrderBook::get_midprice() const
{
    if (bids_.empty() || asks_.empty()) [[unlikely]] {
        return 0.0;
    }
    return (std::prev(bids_.end())->first + asks_.begin()->first) / 2.0;
}

void
LimitOrderBook::change_quantity(tagged_limit_order& order, double quantity_delta)
{
    order.quantity += quantity_delta;
}

void
LimitOrderBook::change_quantity(tagged_market_order& order, double quantity_delta)
{
    order.quantity += quantity_delta;
}

void
LimitOrderBook::change_quantity(order_list::iterator order, double quantity_delta)
{
    if (shared::is_close_to_zero(order->quantity + quantity_delta)) {
        mark_order_removed(order);
        return;
    }

    order->trader->notify_position_change(
        {order->ticker, order->side, quantity_delta, order->price}
    );

    order->quantity += quantity_delta;
}

void
LimitOrderBook::mark_order_removed(order_list::iterator order)
{
    order->trader->notify_position_change(
        {order->ticker, order->side, -order->quantity, order->price}
    );
    if (order->side == shared::Side::buy)
        bids_[order->price].erase(order);
    else
        asks_[order->price].erase(order);
}

LimitOrderBook::order_list::iterator
LimitOrderBook::add_order(const tagged_limit_order& order)
{
    order.trader->notify_position_change(
        {order.ticker, order.side, order.quantity, order.price}
    );

    auto& map = order.side == shared::Side::buy ? bids_ : asks_;

    auto& queue = map[order.price];
    queue.push_back(order);
    return --queue.end();
}

} // namespace nutc::exchange
