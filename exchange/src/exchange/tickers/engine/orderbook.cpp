#include "orderbook.hpp"

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

        if (!q.front().active || q.front().quantity <= 0) {
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
OrderBook::modify_level_(util::Side side, decimal_price price, double quantity)
{
    auto& levels = side == util::Side::buy ? bid_levels_ : ask_levels_;
    levels[price] += quantity;

    if (util::is_close_to_zero(levels[price])) {
        levels.erase(price);
    }

    if (level_update_generator_)
        level_update_generator_->record_level_change(side, price, levels[price]);
}

bool
OrderBook::can_match_orders()
{
	clean_tree(util::Side::sell);
	clean_tree(util::Side::buy);

    if (bids_.empty() || asks_.empty()) {
        return false;
    }
    return get_top_order(util::Side::buy).can_match(get_top_order(util::Side::sell));
}

decimal_price
OrderBook::get_midprice() const
{
    if (bids_.empty() || asks_.empty()) {
        return 0.0;
    }
    return (bids_.begin()->first + std::prev(asks_.end())->first) / 2;
}

double
OrderBook::get_level(util::Side side, decimal_price price) const
{
    const auto& levels = (side == util::Side::buy) ? bid_levels_ : ask_levels_;
    if (!levels.contains(price)) {
        return 0;
    }
    return levels.at(price);
}

std::vector<stored_order>
OrderBook::expire_orders(uint64_t tick)
{
    std::vector<stored_order> result;
    for (auto& [price, q] : bids_) {
        while (!q.empty() && q.front().tick <= tick) {
            result.push_back(std::move(q.front()));
            q.pop();

            modify_level_(util::Side::buy, price, -result.back().quantity);
        }
    }
    for (auto& [price, q] : asks_) {
        while (!q.empty() && q.front().tick <= tick) {
            result.push_back(std::move(q.front()));
            q.pop();

            modify_level_(util::Side::sell, price, -result.back().quantity);
        }
    }
    return result;
}

void
OrderBook::add_order(stored_order order)
{
    order.trader.process_order_add(order);

    if (order.side == util::Side::buy) {
        bids_[order.price].push(order);
    }
    else {
        asks_[order.price].push(order);
    }
    modify_level_(order.side, order.price, order.quantity);
}

} // namespace matching
} // namespace nutc
