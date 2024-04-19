#include "engine.hpp"

#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/util.hpp"

namespace nutc {
namespace matching {

using side = nutc::util::Side;
using ob_update = nutc::messages::orderbook_update;
using match = nutc::messages::match;

namespace {
constexpr double
order_price(const stored_order& order1, const stored_order& order2)
{
    return order1.order_index < order2.order_index ? order1.price : order2.price;
}

constexpr double
order_quantity(const stored_order& order1, const stored_order& order2)
{
    return std::min(order1.quantity, order2.quantity);
}
} // namespace

std::vector<stored_match>
Engine::attempt_matches_()
{
    std::vector<stored_match> matches;
    while (order_container_.can_match_orders()) {
        const stored_order& highest_bid = order_container_.get_top_order(side::buy);
        const stored_order& cheapest_ask = order_container_.get_top_order(side::sell);
        assert(highest_bid.price >= cheapest_ask.price);
        assert(highest_bid.ticker == cheapest_ask.ticker);
        assert(cheapest_ask.quantity > 0);

        if (!order_can_execute_(highest_bid, cheapest_ask))
            continue;

        auto match = build_match(highest_bid, cheapest_ask);
        assert(match.quantity > 0);

        match.buyer->process_order_match(
            match.ticker, util::Side::buy, match.price, match.quantity
        );

        match.seller->process_order_match(
            match.ticker, util::Side::sell, match.price, match.quantity
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

stored_match
Engine::build_match(const stored_order& buyer, const stored_order& seller)
{
    assert(buyer.ticker == seller.ticker);
    double quantity = order_quantity(buyer, seller);
    double price = order_price(buyer, seller);
    util::Side aggressive_side =
        buyer.order_index < seller.order_index ? seller.side : buyer.side;
    return stored_match{
        buyer.trader, seller.trader, buyer.ticker, aggressive_side, price, quantity,
    };
}

bool
Engine::order_can_execute_(const stored_order& buyer, const stored_order& seller)
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

std::vector<stored_match>
Engine::match_order(const stored_order& order)
{
    order_container_.add_order(order);
    return attempt_matches_();
}

} // namespace matching
} // namespace nutc
