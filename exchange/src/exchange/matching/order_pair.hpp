#pragma once

#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/orders/storage/order_storage.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/types/decimal.hpp"
#include "shared/util.hpp"

namespace nutc::exchange {

template <typename... Ts>
concept HasLimitOrder = (is_limit_order_v<Ts> || ...);

template <typename T>
concept IsOrder = std::is_same_v<T, LimitOrderBook::stored_limit_order>
                  || is_limit_order_v<T> || is_market_order_v<T>;

template <typename BuyerT, typename SellerT>
class OrderPair {
    using order_list = std::list<tagged_limit_order>;
    using side = shared::Side;

    BuyerT buyer;
    SellerT seller;

public:
    OrderPair(BuyerT& buyer, SellerT& seller) : buyer(buyer), seller(seller) {}

    template <shared::Side Side>
    IsOrder auto&
    get_order()
    {
        if constexpr (Side == side::buy)
            return buyer;
        else
            return seller;
    }

    template <shared::Side Side>
    const TaggedOrder auto&
    get_underlying_order() const
    {
        if constexpr (Side == shared::Side::buy) {
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

    template <shared::Side Side>
    TaggedOrder auto&
    get_underlying_order()

    {
        if constexpr (Side == shared::Side::buy) {
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

    std::optional<shared::decimal_price>
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

    template <shared::Side AggressiveSide>
    shared::match
    create_match(double quantity, shared::decimal_price price) const
    {
        auto& buyer = get_underlying_order<side::buy>();
        auto& seller = get_underlying_order<side::sell>();
        shared::position position{buyer.ticker, AggressiveSide, quantity, price};
        return {
            position, buyer.trader->get_id(), seller.trader->get_id(),
            buyer.trader->get_capital(), seller.trader->get_capital()
        };
    }

    double
    potential_match_quantity() const
    {
        auto& buyer = get_underlying_order<side::buy>();
        auto& seller = get_underlying_order<side::sell>();
        return std::min(buyer.quantity, seller.quantity);
    }

    void
    handle_match(
        const shared::match& match, shared::decimal_price order_fee,
        LimitOrderBook& orderbook
    )
    {
        get_underlying_order<side::buy>().trader->notify_match(
            {match.position.ticker, shared::Side::buy, match.position.quantity,
             match.position.price * (shared::decimal_price{1.0} + order_fee)}
        );
        get_underlying_order<side::sell>().trader->notify_match(
            {match.position.ticker, shared::Side::sell, match.position.quantity,
             match.position.price * (shared::decimal_price{1.0} - order_fee)}
        );

        orderbook.change_quantity(seller, -match.position.quantity);
        orderbook.change_quantity(buyer, -match.position.quantity);
    }
};
} // namespace nutc::exchange
