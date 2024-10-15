#include "limit_orderbook.hpp"

#include "common/types/decimal.hpp"
#include "common/util.hpp"
#include "exchange/orders/storage/order_storage.hpp"
#include <iostream>

namespace nutc::exchange {

std::optional<LimitOrderBook::order_list::iterator>
LimitOrderBook::get_top_order(common::Side side)
{
    auto& tree = side == common::Side::buy ? bids_ : asks_;

    if (tree.empty()) [[unlikely]]
        return std::nullopt;

    auto key = side == common::Side::buy ? std::prev(tree.end()) : tree.begin();
    auto& queue = key->second;
    assert(!queue.empty());

    return queue.begin();
}

common::decimal_price
LimitOrderBook::get_midprice() const
{
    if (bids_.empty() || asks_.empty()) [[unlikely]] {
        return 0.0;
    }
    return (std::prev(bids_.end())->first + asks_.begin()->first) / 2.0;
}

void
LimitOrderBook::change_quantity(
    order_list::iterator order, common::decimal_quantity quantity_delta
)
{
    order->trader->get_portfolio().notify_position_change(
        {order->ticker, order->side, quantity_delta, order->price}
    );

    order->quantity += quantity_delta;
    assert(order->quantity != 0.0);
}

void
LimitOrderBook::remove_order(order_list::iterator order)
{
    order->trader->get_portfolio().notify_position_change(
        {order->ticker, order->side, -order->quantity, order->price}
    );
    if (order->side == common::Side::buy)
        remove_order(order, bids_);
    else
        remove_order(order, asks_);
}

void
LimitOrderBook::remove_order(
    order_list::iterator order, std::map<common::decimal_price, order_list>& map
)
{
    auto list_it = map.find(order->price);
    assert(list_it != map.end());
    list_it->second.erase(order);

    if (list_it->second.size() == 0) [[unlikely]]
        map.erase(list_it);
}

LimitOrderBook::order_list::iterator
LimitOrderBook::add_order(const tagged_limit_order& order)
{
    order.trader->get_portfolio().notify_position_change(
        {order.ticker, order.side, order.quantity, order.price}
    );

    auto& map = order.side == common::Side::buy ? bids_ : asks_;

    auto& queue = map[order.price];
    queue.push_back(order);
    return std::prev(queue.end());
}

} // namespace nutc::exchange
