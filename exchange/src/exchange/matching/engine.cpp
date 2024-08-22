#include "engine.hpp"

#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/util.hpp"

namespace nutc {
namespace matching {

using match = nutc::messages::match;

template <TaggedOrder OrderT>
std::vector<match>
Engine::match_order(OrderT order, LimitOrderBook& orderbook)
{
    std::vector<match> matches;

    while (order.active) {
        auto match_opt = match_incoming_order_(order, orderbook);
        if (match_opt.has_value())
            matches.push_back(match_opt.value());
        else if (match_opt.error())
            break;
    }

    if constexpr (is_limit_order_v<OrderT>) {
        if (!order.ioc && !util::is_close_to_zero(order.quantity)) {
            orderbook.add_order(order);
        }
    }

    return matches;
}

template <TaggedOrder BuyerT, TaggedOrder SellerT>
requires HasLimitOrder<BuyerT, SellerT>
std::optional<util::decimal_price>
Engine::potential_match_price(const BuyerT& buyer, const SellerT& seller)
{
    if constexpr (is_market_order_v<decltype(buyer)>)
        return seller.price;
    else if constexpr (is_market_order_v<decltype(seller)>)
        return buyer.price;
    else if (buyer.price < seller.price)
        return std::nullopt;
    else
        return buyer.timestamp < seller.timestamp ? buyer.price : seller.price;
}

template <util::Side AggressiveSide, TaggedOrder BuyerT, TaggedOrder SellerT>
requires HasLimitOrder<BuyerT, SellerT>
glz::expected<match, bool>
Engine::match_orders_(BuyerT& buyer, SellerT& seller, LimitOrderBook& orderbook)
{
    auto match_result = attempt_match_<AggressiveSide>(buyer, seller);
    if (match_result.has_value()) [[likely]] {
        if constexpr (is_limit_order_v<decltype(seller)>) {
            orderbook.change_quantity(seller, -match_result->position.quantity);
        }
        else {
            seller.active = false;
        }
        if constexpr (is_limit_order_v<decltype(buyer)>) {
            orderbook.change_quantity(buyer, -match_result->position.quantity);
        }
        else {
            buyer.active = false;
        }
        return match_result.value();
    }
    if (match_result.error() == MatchFailure::seller_failure) {
        if constexpr (is_limit_order_v<decltype(seller)>) {
            orderbook.mark_order_removed(seller);
        }
        if constexpr (AggressiveSide == side::buy)
            return glz::unexpected(false);
    }
    if (match_result.error() == MatchFailure::buyer_failure) {
        if constexpr (is_limit_order_v<decltype(buyer)>) {
            orderbook.mark_order_removed(buyer);
        }
        if constexpr (AggressiveSide == side::sell)
            return glz::unexpected(false);
    }
    return glz::unexpected(true);
}

template <util::Side AggressiveSide, TaggedOrder OrderT>
glz::expected<match, bool>
Engine::match_incoming_order_(
    OrderT& aggressive_order, tagged_limit_order& passive_order,
    LimitOrderBook& orderbook
)
{
    if constexpr (AggressiveSide == side::buy) {
        return match_orders_<AggressiveSide>(
            aggressive_order, passive_order, orderbook
        );
    }
    else {
        return match_orders_<AggressiveSide>(
            passive_order, aggressive_order, orderbook
        );
    }
}

template <TaggedOrder BuyerT, TaggedOrder SellerT>
double
Engine::potential_match_quantity(const BuyerT& order1, const SellerT& order2)
{
    return std::min(order1.quantity, order2.quantity);
}

util::decimal_price
Engine::total_order_cost_(decimal_price price, double quantity) const
{
    decimal_price fee{order_fee_ * price};
    decimal_price price_per = price + fee;
    return price_per * quantity;
}

template <util::Side AggressiveSide, TaggedOrder BuyerT, TaggedOrder SellerT>
requires HasLimitOrder<BuyerT, SellerT>
glz::expected<match, Engine::MatchFailure>
Engine::attempt_match_(BuyerT& buyer, SellerT& seller)
{
    auto price_opt = potential_match_price(buyer, seller);
    if (!price_opt) [[unlikely]]
        return glz::unexpected(MatchFailure::done_matching);

    auto match_price = *price_opt;
    double match_quantity = potential_match_quantity(buyer, seller);
    decimal_price total_price{total_order_cost_(match_price, match_quantity)};
    if (buyer.trader->get_capital() < total_price) [[unlikely]]
        return glz::unexpected(MatchFailure::buyer_failure);
    if (seller.trader->get_holdings(seller.ticker) < match_quantity) [[unlikely]]
        return glz::unexpected(MatchFailure::seller_failure);
    if (buyer.trader == seller.trader) [[unlikely]] {
        return glz::unexpected(MatchFailure::buyer_failure);
    }
    return create_match_(match_price, match_quantity, buyer, seller);
}

template <TaggedOrder OrderT>
glz::expected<match, bool>
Engine::match_incoming_order_(OrderT& aggressive_order, LimitOrderBook& orderbook)
{
    if (aggressive_order.side == side::buy) {
        auto passive_order = orderbook.get_top_order(side::sell);
        if (!passive_order.has_value())
            return glz::unexpected(true);
        return match_incoming_order_<side::buy>(
            aggressive_order, *passive_order, orderbook
        );
    }

    assert(aggressive_order.side == side::sell);
    auto passive_order = orderbook.get_top_order(side::buy);
    if (!passive_order.has_value())
        return glz::unexpected(true);
    return match_incoming_order_<side::sell>(
        aggressive_order, *passive_order, orderbook
    );
}

template <TaggedOrder BuyerT, TaggedOrder SellerT>
requires HasLimitOrder<BuyerT, SellerT>
messages::match
Engine::create_match_(
    util::decimal_price price, double quantity, BuyerT& buyer, SellerT& seller
)
{
    util::Side aggressive_side =
        buyer.timestamp < seller.timestamp ? util::Side::sell : util::Side::buy;

    auto& ticker = buyer.ticker;
    buyer.trader->notify_match(
        {ticker, util::Side::buy, quantity,
         price * (util::decimal_price{1.0} + order_fee_)}
    );
    seller.trader->notify_match(
        {ticker, util::Side::sell, quantity,
         price * (util::decimal_price{1.0} - order_fee_)}
    );

    util::position position{buyer.ticker, aggressive_side, quantity, price};
    return {
        position, buyer.trader->get_id(), seller.trader->get_id(),
        buyer.trader->get_capital(), seller.trader->get_capital()
    };
}

template std::vector<match> Engine::match_order<>(tagged_limit_order, LimitOrderBook&);

template std::vector<match> Engine::match_order<>(tagged_market_order, LimitOrderBook&);

} // namespace matching
} // namespace nutc
