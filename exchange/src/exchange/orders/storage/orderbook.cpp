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

        if (!q.front().isActive || util::is_close_to_zero(q.front().quantity)) {
            auto order = q.front();
            order.trader.process_order_remove(order);
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
OrderBook::expire_orders(uint64_t tick)
{
    auto remove_order = [this](util::Side side, double price, auto& queue) {
        auto order = queue.front();
        order.trader.process_order_remove(order);
        modify_level_(side, price, -order.quantity);
        queue.pop();

        return order;
    };

    std::vector<stored_order> result;
    for (auto& [price, q] : bids_) {
        while (!q.empty() && q.front().tick <= tick) {
            result.emplace_back(remove_order(util::Side::buy, price, q));
        }
    }
    for (auto& [price, q] : asks_) {
        while (!q.empty() && q.front().tick <= tick) {
            result.emplace_back(remove_order(util::Side::sell, price, q));
        }
    }
    return result;
}

void
OrderBook::add_order(stored_order order)
{
    order.trader.process_order_add(order);

    auto& map = order.side == util::Side::buy ? bids_ : asks_;
    map[order.price].push(order);

    modify_level_(order.side, order.price, order.quantity);
}

} // namespace matching
} // namespace nutc
