#include "engine.hpp"

#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/util.hpp"

namespace nutc::exchange {

using match = nutc::shared::match;

template <TaggedOrder OrderT>
std::vector<match>
Engine::match_order(OrderT order, LimitOrderBook& orderbook)
{
    std::vector<match> matches;

    while (!shared::is_close_to_zero(order.quantity)) {
        auto match_opt = match_incoming_order_(order, orderbook);
        if (match_opt.has_value())
            matches.push_back(match_opt.value());
        else if (match_opt.error())
            break;
    }

    if constexpr (is_limit_order_v<OrderT>) {
        if (!order.ioc && !shared::is_close_to_zero(order.quantity)) {
            orderbook.add_order(order);
        }
    }

    return matches;
}

template <shared::Side AggressiveSide, typename OrderPairT>
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
            orderbook.mark_order_removed(orders.template get_order<shared::Side::sell>()
            );
            return glz::unexpected(false);
        }
    }
    if (match_result.error() == MatchFailure::buyer_failure) {
        if constexpr (AggressiveSide == side::sell) {
            orderbook.mark_order_removed(orders.template get_order<shared::Side::buy>()
            );
            return glz::unexpected(false);
        }
    }
    return glz::unexpected(true);
}

template <shared::Side AggressiveSide, TaggedOrder OrderT>
glz::expected<match, bool>
Engine::match_incoming_order_(
    OrderT& aggressive_order, LimitOrderBook::stored_limit_order passive_order,
    LimitOrderBook& orderbook
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

shared::decimal_price
Engine::total_order_cost_(decimal_price price, double quantity) const
{
    decimal_price fee{order_fee_ * price};
    decimal_price price_per = price + fee;
    return price_per * quantity;
}

template <shared::Side AggressiveSide, typename OrderPairT>
glz::expected<match, Engine::MatchFailure>
Engine::attempt_match_(OrderPairT& orders)
{
    auto price_opt = orders.potential_match_price();
    if (!price_opt) [[unlikely]]
        return glz::unexpected(MatchFailure::done_matching);

    auto match_price = *price_opt;
    double match_quantity = orders.potential_match_quantity();
    decimal_price total_price{total_order_cost_(match_price, match_quantity)};

    auto& buyer = orders.template get_underlying_order<shared::Side::buy>();
    auto& seller = orders.template get_underlying_order<shared::Side::sell>();

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
Engine::match_incoming_order_(OrderT& aggressive_order, LimitOrderBook& orderbook)
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

template std::vector<match> Engine::match_order<>(tagged_limit_order, LimitOrderBook&);

template std::vector<match> Engine::match_order<>(tagged_market_order, LimitOrderBook&);

} // namespace nutc::exchange
