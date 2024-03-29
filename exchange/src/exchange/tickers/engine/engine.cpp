#include "engine.hpp"

#include "exchange/traders/trader_manager.hpp"

namespace nutc {
namespace matching {
constexpr auto
order_price(const StoredOrder& order1, const StoredOrder& order2)
{
    return order1.order_index < order2.order_index ? order1.price : order2.price;
}

constexpr double
order_quantity(const StoredOrder& order1, const StoredOrder& order2)
{
    return std::min(order1.quantity, order2.quantity);
}

std::vector<StoredMatch>
Engine::attempt_matches_()
{
    std::vector<StoredMatch> matches;
    while (order_container_.can_match_orders()) {
        const StoredOrder& highest_bid = order_container_.get_top_order(SIDE::BUY);
        const StoredOrder& cheapest_ask = order_container_.get_top_order(SIDE::SELL);
        assert(highest_bid.price >= cheapest_ask.price);
        assert(highest_bid.ticker == cheapest_ask.ticker);
        assert(cheapest_ask.price > 0);

        if (!order_can_execute_(highest_bid, cheapest_ask))
            continue;

        auto match = build_match(highest_bid, cheapest_ask);
        assert(match.quantity > 0);
        match.buyer->process_order_match(
            match.ticker, messages::SIDE::BUY, match.price, match.quantity
        );
        match.seller->process_order_match(
            match.ticker, messages::SIDE::SELL, match.price, match.quantity
        );
        matches.push_back(std::move(match));
        order_container_.modify_order_quantity(
            highest_bid.order_index, -match.quantity
        );
        order_container_.modify_order_quantity(
            cheapest_ask.order_index, -match.quantity
        );
    }
    return matches;
}

StoredMatch
Engine::build_match(const StoredOrder& buyer, const StoredOrder& seller)
{
    assert(buyer.ticker == seller.ticker);
    double quantity = order_quantity(buyer, seller);
    double price = order_price(buyer, seller);
    SIDE aggressive_side =
        buyer.order_index < seller.order_index ? seller.side : buyer.side;
    return StoredMatch{
        buyer.trader, seller.trader, buyer.ticker, aggressive_side, price, quantity,
    };
}

bool
Engine::order_can_execute_(const StoredOrder& buyer, const StoredOrder& seller)
{
    double quantity = order_quantity(buyer, seller);
    double price = order_price(buyer, seller);
    if (!buyer.trader->can_leverage()
        && buyer.trader->get_capital() < price * quantity) {
        order_container_.remove_order(buyer.order_index);
        return false;
    }
    if (!seller.trader->can_leverage()
        && seller.trader->get_holdings(seller.ticker) < quantity) {
        order_container_.remove_order(seller.order_index);
        return false;
    }
    return true;
}

std::vector<StoredMatch>
Engine::match_order(const MarketOrder& order)
{
    auto trader = manager::TraderManager::get_instance().get_trader(order.client_id);
    if (order.quantity <= 0 || order.price <= 0)
        return {};
    // round to 2 decimal places
    // TODO: make this configurable. right now, it just rounds to one cent
    double price = std::round(order.price * 100) / 100;
    order_container_.add_order(StoredOrder{
        trader, order.side, order.ticker, order.quantity, price, current_tick_
    });
    return attempt_matches_();
}

} // namespace matching
} // namespace nutc
