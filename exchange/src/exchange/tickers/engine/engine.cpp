#include "engine.hpp"

#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/util.hpp"

namespace nutc {
namespace matching {

using side = nutc::util::Side;
using ob_update = nutc::messages::orderbook_update;
using match = nutc::messages::match;

namespace {
static constexpr decimal_price decimal_one(1.0);

constexpr decimal_price
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
Engine::attempt_matches_(OrderBook& orderbook)
{
    std::vector<stored_match> matches;
    while (orderbook.can_match_orders()) {
        const stored_order& highest_bid = orderbook.get_top_order(side::buy);
        const stored_order& cheapest_ask = orderbook.get_top_order(side::sell);
        assert(highest_bid.price >= cheapest_ask.price);
        assert(highest_bid.ticker == cheapest_ask.ticker);
        assert(cheapest_ask.quantity > 0);

        if (!order_can_execute_(orderbook, highest_bid, cheapest_ask))
            continue;

        auto match = create_match(highest_bid, cheapest_ask);

        orderbook.modify_order_quantity(highest_bid.order_index, -match.quantity);
        orderbook.modify_order_quantity(cheapest_ask.order_index, -match.quantity);

        matches.push_back(std::move(match));
    }
    return matches;
}

stored_match
Engine::create_match(const stored_order& buyer, const stored_order& seller)
{
    assert(buyer.ticker == seller.ticker);
    double quantity = order_quantity(buyer, seller);
    decimal_price price = order_price(buyer, seller);
    util::Side aggressive_side =
        buyer.order_index < seller.order_index ? util::Side::sell : util::Side::buy;

    stored_match match{
        buyer.trader, seller.trader, buyer.ticker, aggressive_side, price, quantity,
    };

    match.buyer.process_order_match(
        {util::Side::buy, match.ticker, match.quantity,
         match.price * (decimal_one + order_fee)}
    );
    match.seller.process_order_match(
        {util::Side::sell, match.ticker, match.quantity,
         match.price * (decimal_one - order_fee)}
    );
    return match;
}

bool
Engine::order_can_execute_(
    OrderBook& orderbook, const stored_order& buyer, const stored_order& seller
)
{
    double quantity = order_quantity(buyer, seller);

    decimal_price price = order_price(buyer, seller);
    double total_price = double((decimal_one + order_fee) * price) * quantity;

    if (!buyer.trader.can_leverage() && buyer.trader.get_capital() < total_price) {
        orderbook.remove_order(buyer.order_index);
        return false;
    }
    if (!seller.trader.can_leverage()
        && seller.trader.get_holdings(seller.ticker) < quantity) {
        orderbook.remove_order(seller.order_index);
        return false;
    }
    if (&seller.trader == &buyer.trader) [[unlikely]] {
        orderbook.remove_order(std::min(seller.order_index, buyer.order_index));
        return false;
    }
    return true;
}

std::vector<stored_match>
Engine::match_order(OrderBook& orderbook, const stored_order& order)
{
    orderbook.add_order(order);
    return attempt_matches_(orderbook);
}

} // namespace matching
} // namespace nutc
