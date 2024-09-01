#include "engine.hpp"

#include "common/messages_exchange_to_wrapper.hpp"
#include "common/types/decimal.hpp"
#include "common/util.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "order_pair.hpp"

namespace nutc::exchange {

using match = nutc::common::match;

template <TaggedOrder OrderT>
std::vector<match>
Engine::match_order(OrderT order, CompositeOrderBook& orderbook)
{
    std::vector<match> matches;

    while (order.quantity != 0.0) {
        auto match_opt = match_incoming_order_(order, orderbook);
        if (match_opt.has_value())
            matches.push_back(match_opt.value());
        else if (match_opt.error())
            break;
    }

    if constexpr (is_limit_order_v<OrderT>) {
        if (!(order.ioc || order.quantity == 0.0)) {
            orderbook.add_order(order);
        }
    }

    return matches;
}

template <common::Side AggressiveSide, typename OrderPairT>
glz::expected<match, bool>
Engine::match_orders_(OrderPairT& orders, CompositeOrderBook& orderbook)
{
    auto match_result = attempt_match_<AggressiveSide>(orders);
    if (match_result.has_value()) [[likely]] {
        orders.handle_match(*match_result, order_fee_, orderbook);
        return match_result.value();
    }
    if (match_result.error() == MatchFailure::seller_failure) {
        if constexpr (AggressiveSide == side::buy) {
            orderbook.remove_order(orders.template get_order<common::Side::sell>());
            return glz::unexpected(false);
        }
    }
    if (match_result.error() == MatchFailure::buyer_failure) {
        if constexpr (AggressiveSide == side::sell) {
            orderbook.remove_order(orders.template get_order<common::Side::buy>());
            return glz::unexpected(false);
        }
    }
    return glz::unexpected(true);
}

template <common::Side AggressiveSide, TaggedOrder OrderT>
glz::expected<match, bool>
Engine::match_incoming_order_(
    OrderT& aggressive_order, LimitOrderBook::stored_limit_order passive_order,
    CompositeOrderBook& orderbook
)
{
    // We can copy stored_limit_order because it's already pointing to the order. We
    // cannot copy OrderT by value because it will then point to something else
    // This is a confusing micro optimization. Trust tho.
    if constexpr (AggressiveSide == side::buy) {
        OrderPair<OrderT&, LimitOrderBook::stored_limit_order> pair{
            aggressive_order, passive_order
        };
        return match_orders_<AggressiveSide>(pair, orderbook);
    }
    else {
        OrderPair<LimitOrderBook::stored_limit_order, OrderT&> pair{
            passive_order, aggressive_order
        };
        return match_orders_<AggressiveSide>(pair, orderbook);
    }
}

common::decimal_price
Engine::total_order_cost_(decimal_price price, common::decimal_quantity quantity) const
{
    decimal_price fee{order_fee_ * price};
    decimal_price price_per = price + fee;
    return price_per * quantity;
}

template <common::Side AggressiveSide, typename OrderPairT>
glz::expected<match, Engine::MatchFailure>
Engine::attempt_match_(OrderPairT& orders)
{
    auto price_opt = orders.potential_match_price();
    if (!price_opt) [[unlikely]]
        return glz::unexpected(MatchFailure::done_matching);

    auto match_price = *price_opt;
    common::decimal_quantity match_quantity = orders.potential_match_quantity();
    decimal_price total_price{total_order_cost_(match_price, match_quantity)};

    auto& buyer = orders.template get_underlying_order<common::Side::buy>();
    auto& seller = orders.template get_underlying_order<common::Side::sell>();

    if (buyer.trader->get_capital() < total_price) [[unlikely]]
        return glz::unexpected(MatchFailure::buyer_failure);
    if (seller.trader->get_holdings(buyer.ticker) < match_quantity) [[unlikely]]
        return glz::unexpected(MatchFailure::seller_failure);
    if (buyer.trader == seller.trader) [[unlikely]] {
        return glz::unexpected(MatchFailure::buyer_failure);
    }
    return orders.template create_match<AggressiveSide>(match_quantity, match_price);
}

template <TaggedOrder OrderT>
glz::expected<match, bool>
Engine::match_incoming_order_(OrderT& aggressive_order, CompositeOrderBook& orderbook)
{
    if (aggressive_order.side == side::buy) {
        auto passive_order = orderbook.get_top_order(side::sell);
        if (!passive_order.has_value())
            return glz::unexpected(true);
        return match_incoming_order_<side::buy>(
            aggressive_order, passive_order.value(), orderbook
        );
    }

    auto passive_order = orderbook.get_top_order(side::buy);
    if (!passive_order.has_value())
        return glz::unexpected(true);
    return match_incoming_order_<side::sell>(
        aggressive_order, passive_order.value(), orderbook
    );
}

template std::vector<match>
Engine::match_order<>(tagged_limit_order, CompositeOrderBook&);

template std::vector<match>
Engine::match_order<>(tagged_market_order, CompositeOrderBook&);

} // namespace nutc::exchange
