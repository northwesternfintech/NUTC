#pragma once

#include "common/messages_exchange_to_wrapper.hpp"
#include "common/types/decimal.hpp"
#include "common/util.hpp"
#include "exchange/orders/orderbook/composite_orderbook.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/orders/storage/order_storage.hpp"

namespace nutc::exchange {

template <typename... Ts>
concept HasLimitOrder = (is_limit_order_v<Ts> || ...);

template <typename T>
concept IsOrder = std::is_same_v<T, LimitOrderBook::stored_limit_order>
                  || is_limit_order_v<T> || is_market_order_v<T>;

template <typename BuyerT, typename SellerT>
class OrderPair {
    using order_list = std::list<tagged_limit_order>;
    using side = common::Side;

    BuyerT buyer;
    SellerT seller;

public:
    OrderPair(BuyerT& buyer, SellerT& seller) : buyer(buyer), seller(seller) {}

    template <common::Side Side>
    IsOrder auto&
    get_order()
    {
        if constexpr (Side == side::buy)
            return buyer;
        else
            return seller;
    }

    template <common::Side Side>
    const TaggedOrder auto&
    get_underlying_order() const
    {
        if constexpr (Side == common::Side::buy) {
            if constexpr (is_stored_limit_order_v<BuyerT>) {
                return *buyer;
            }
            else {
                return buyer;
            }
        }

        else {
            if constexpr (is_stored_limit_order_v<SellerT>) {
                return *seller;
            }
            else {
                return seller;
            }
        }
    }

    template <common::Side Side>
    TaggedOrder auto&
    get_underlying_order()

    {
        if constexpr (Side == common::Side::buy) {
            if constexpr (is_stored_limit_order_v<BuyerT>) {
                return *buyer;
            }
            else {
                return buyer;
            }
        }
        else {
            if constexpr (is_stored_limit_order_v<SellerT>) {
                return *seller;
            }
            else {
                return seller;
            }
        }
    }

    std::optional<common::decimal_price>
    potential_match_price() const

    {
        auto& buyer_v = get_underlying_order<side::buy>();
        auto& seller_v = get_underlying_order<side::sell>();
        if constexpr (is_market_order_v<BuyerT>)
            return seller_v.price;
        else if constexpr (is_market_order_v<SellerT>)
            return buyer_v.price;
        else if (buyer_v.price < seller_v.price)
            return std::nullopt;
        else
            return buyer_v.timestamp < seller_v.timestamp ? buyer_v.price
                                                          : seller_v.price;
    }

    template <common::Side AggressiveSide>
    common::match
    create_match(common::decimal_quantity quantity, common::decimal_price price) const
    {
        auto& buyer = get_underlying_order<side::buy>();
        auto& seller = get_underlying_order<side::sell>();
        common::position position{buyer.ticker, AggressiveSide, quantity, price};
        // TODO: match_type is pretty bad, we should have a better way of tracking this.
        // It's only used for metrics
        std::string match_type =
            fmt::format("{}->{}", seller.trader->get_type(), buyer.trader->get_type());
        // TODO: can just use TraderPortfolio instead of entire trader
        common::match match{
            position, buyer.trader->get_id(), seller.trader->get_id(),
            buyer.trader->get_portfolio().get_capital(),
            seller.trader->get_portfolio().get_capital()
        };
        match.match_type = match_type;
        return match;
    }

    common::decimal_quantity
    potential_match_quantity() const
    {
        auto& buyer = get_underlying_order<side::buy>();
        auto& seller = get_underlying_order<side::sell>();
        return std::min(buyer.quantity, seller.quantity);
    }

    void
    handle_match(
        const common::match& match, common::decimal_price order_fee,
        CompositeOrderBook& orderbook
    )
    {
        get_underlying_order<side::buy>().trader->get_portfolio().notify_match(
            {match.position.ticker, common::Side::buy, match.position.quantity,
             match.position.price * (common::decimal_price{1.0} + order_fee)}
        );
        get_underlying_order<side::sell>().trader->get_portfolio().notify_match(
            {match.position.ticker, common::Side::sell, match.position.quantity,
             match.position.price * (common::decimal_price{1.0} - order_fee)}
        );

        change_order_quantity(seller, -match.position.quantity, orderbook);
        change_order_quantity(buyer, -match.position.quantity, orderbook);
    }

private:
    template <IsOrder OrderT>
    static void
    change_order_quantity(
        OrderT& order, common::decimal_quantity quantity_delta,
        CompositeOrderBook& orderbook
    )
    {
        if constexpr (is_stored_limit_order_v<OrderT>) {
            orderbook.change_quantity(order, quantity_delta);
        }
        else {
            order.quantity += quantity_delta;
        }
    }
};
} // namespace nutc::exchange
