#include "engine.hpp"

#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/util.hpp"

namespace nutc {
namespace matching {

using side = nutc::util::Side;
using match = nutc::messages::match;

namespace {
static constexpr util::decimal_price decimal_one(1.0);

constexpr util::decimal_price
order_price(const tagged_limit_order& order1, const tagged_limit_order& order2)
{
    return order1.order_index < order2.order_index ? order1.price : order2.price;
}

constexpr double
order_quantity(const tagged_limit_order& order1, const tagged_limit_order& order2)
{
    return std::min(order1.quantity, order2.quantity);
}
} // namespace

std::vector<stored_match>
Engine::match_orders(LimitOrderBook& orderbook)
{
    std::vector<stored_match> matches;
    while (true) {
        auto highest_bid_opt = orderbook.get_top_order(side::buy);
        auto cheapest_ask_opt = orderbook.get_top_order(side::sell);

        if (!highest_bid_opt.has_value() || !cheapest_ask_opt.has_value())
            [[unlikely]] {
            break;
        }

        tagged_limit_order& highest_bid = highest_bid_opt->get();
        tagged_limit_order& cheapest_ask = cheapest_ask_opt->get();

        if (highest_bid.price < cheapest_ask.price) [[unlikely]] {
            break;
        }

        if (!order_can_execute_(orderbook, highest_bid, cheapest_ask))
            continue;

        auto match = create_match(highest_bid, cheapest_ask);

        orderbook.change_quantity(highest_bid, -match.position.quantity);
        orderbook.change_quantity(cheapest_ask, -match.position.quantity);

        matches.emplace_back(match);
    }
    return matches;
}

stored_match
Engine::create_match(const tagged_limit_order& buyer, const tagged_limit_order& seller)
{
    assert(buyer.ticker == seller.ticker);
    double quantity = order_quantity(buyer, seller);
    util::decimal_price price = order_price(buyer, seller);

    util::Side aggressive_side =
        buyer.order_index < seller.order_index ? util::Side::sell : util::Side::buy;

    auto& ticker = buyer.ticker;
    buyer.trader->process_order_match(
        {ticker, util::Side::buy, quantity, price * (decimal_one + order_fee)}
    );
    seller.trader->process_order_match(
        {ticker, util::Side::sell, quantity, price * (decimal_one - order_fee)}
    );

    util::position position{buyer.ticker, aggressive_side, quantity, price};
    stored_match match{*buyer.trader, *seller.trader, position};
    return match;
}

bool
Engine::order_can_execute_(
    LimitOrderBook& orderbook, tagged_limit_order& buyer, tagged_limit_order& seller
)
{
    double quantity = order_quantity(buyer, seller);

    util::decimal_price price = order_price(buyer, seller);
    util::decimal_price total_price = ((decimal_one + order_fee) * price) * quantity;

    if (!buyer.trader->can_leverage() && buyer.trader->get_capital() < total_price) {
        orderbook.mark_order_removed(buyer);
        return false;
    }
    if (!seller.trader->can_leverage()
        && seller.trader->get_holdings(seller.ticker) < quantity) {
        orderbook.mark_order_removed(seller);
        return false;
    }
    if (seller.trader == buyer.trader) [[unlikely]] {
        if (seller.order_index <= buyer.order_index) {
            orderbook.mark_order_removed(seller);
        }
        else {
            orderbook.mark_order_removed(buyer);
        }
        return false;
    }

    return true;
}

} // namespace matching
} // namespace nutc
