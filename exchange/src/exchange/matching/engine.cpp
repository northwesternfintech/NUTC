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

template <util::Side AggressiveSide, typename OrderPairT>
glz::expected<match, bool>
Engine::match_orders_(OrderPairT& orders, LimitOrderBook& orderbook)
{
    auto match_result = attempt_match_<AggressiveSide>(orders);
    if (match_result.has_value()) [[likely]] {
        orders.handle_match(*match_result, order_fee_, orderbook);
        return match_result.value();
    }
    if (match_result.error() == MatchFailure::seller_failure) {
        if constexpr (AggressiveSide == side::buy) {
            orderbook.mark_order_removed(orders.seller);
            return glz::unexpected(false);
        }
    }
    if (match_result.error() == MatchFailure::buyer_failure) {
        if constexpr (AggressiveSide == side::sell) {
            orderbook.mark_order_removed(orders.buyer);
            return glz::unexpected(false);
        }
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
        OrderPair pair{aggressive_order, passive_order};
        return match_orders_<AggressiveSide>(pair, orderbook);
    }
    else {
        OrderPair pair{passive_order, aggressive_order};
        return match_orders_<AggressiveSide>(pair, orderbook);
    }
}

util::decimal_price
Engine::total_order_cost_(decimal_price price, double quantity) const
{
    decimal_price fee{order_fee_ * price};
    decimal_price price_per = price + fee;
    return price_per * quantity;
}

template <util::Side AggressiveSide, typename OrderPairT>
glz::expected<match, Engine::MatchFailure>
Engine::attempt_match_(OrderPairT& orders)
{
    auto price_opt = orders.potential_match_price();
    if (!price_opt) [[unlikely]]
        return glz::unexpected(MatchFailure::done_matching);

    auto match_price = *price_opt;
    double match_quantity = orders.potential_match_quantity();
    decimal_price total_price{total_order_cost_(match_price, match_quantity)};
    if (orders.buyer.trader->get_capital() < total_price) [[unlikely]]
        return glz::unexpected(MatchFailure::buyer_failure);
    if (orders.seller.trader->get_holdings(orders.seller.ticker) < match_quantity)
        [[unlikely]]
        return glz::unexpected(MatchFailure::seller_failure);
    if (orders.buyer.trader == orders.seller.trader) [[unlikely]] {
        return glz::unexpected(MatchFailure::buyer_failure);
    }
    return orders.template create_match<AggressiveSide>(match_quantity, match_price);
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

    auto passive_order = orderbook.get_top_order(side::buy);
    if (!passive_order.has_value())
        return glz::unexpected(true);
    return match_incoming_order_<side::sell>(
        aggressive_order, *passive_order, orderbook
    );
}

template std::vector<match> Engine::match_order<>(tagged_limit_order, LimitOrderBook&);

template std::vector<match> Engine::match_order<>(tagged_market_order, LimitOrderBook&);

} // namespace matching
} // namespace nutc
