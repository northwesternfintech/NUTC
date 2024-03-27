#include "engine.hpp"

#include "exchange/traders/trader_manager.hpp"

namespace nutc {
namespace matching {
std::vector<StoredMatch>
Engine::attempt_matches_()
{
    std::vector<StoredMatch> matches;
    while (order_container_.can_match_orders()) {
        const StoredOrder& highest_bid = order_container_.get_top_order(SIDE::BUY);
        const StoredOrder& cheapest_ask = order_container_.get_top_order(SIDE::SELL);
        assert(highest_bid.price >= cheapest_ask.price);
        assert(highest_bid.ticker == cheapest_ask.ticker);

        if (handle_order_failure_(highest_bid, cheapest_ask))
            continue;

        double quantity = std::min(highest_bid.quantity, cheapest_ask.quantity);

        matches.push_back(build_match(highest_bid, cheapest_ask));
        order_container_.modify_order_quantity(highest_bid.order_index, -quantity);
        order_container_.modify_order_quantity(cheapest_ask.order_index, -quantity);
    }
    return matches;
}

StoredMatch
Engine::build_match(const StoredOrder& buyer, const StoredOrder& seller)
{
    double quantity = std::min(buyer.quantity, seller.quantity);
    double price = buyer.order_index < seller.order_index ? buyer.price : seller.price;
    SIDE aggressive_side =
        buyer.order_index < seller.order_index ? seller.side : buyer.side;
    return StoredMatch{
        buyer.trader, seller.trader, buyer.ticker, aggressive_side, price, quantity,
    };
}

bool
Engine::handle_order_failure_(const StoredOrder& buyer, const StoredOrder& seller)
{
    double quantity = std::min(buyer.quantity, seller.quantity);
    double price = buyer.order_index < seller.order_index ? buyer.price : seller.price;
    if (!buyer.trader->can_leverage()
        && buyer.trader->get_capital() < price * quantity) {
        order_container_.remove_order(buyer.order_index);
        return true;
    }
    if (!seller.trader->can_leverage()
        && seller.trader->get_holdings(buyer.ticker) < quantity) {
        order_container_.remove_order(seller.order_index);
        return true;
    }
    return false;
}

std::vector<StoredMatch>
Engine::match_order(const MarketOrder& order)
{
    auto trader = manager::TraderManager::get_instance().get_trader(order.client_id);
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
