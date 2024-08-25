#pragma once

#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/orders/storage/order_storage.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/types/decimal_price.hpp"
#include "shared/util.hpp"

namespace nutc::matching {
template <typename... Ts>
concept HasLimitOrder = std::disjunction_v<is_limit_order<Ts>...>;

template <TaggedOrder BuyerT, TaggedOrder SellerT>
requires HasLimitOrder<BuyerT, SellerT>
class OrderPair {
public:
    BuyerT& buyer;
    SellerT& seller;

    OrderPair(BuyerT& buyer, SellerT& seller) : buyer(buyer), seller(seller) {}

    void handle_match(
        const messages::match& match, util::decimal_price order_fee,
        LimitOrderBook& orderbook
    );

    template <util::Side AggressiveSide>
    messages::match
    create_match(double quantity, util::decimal_price price) const
    {
        util::position position{buyer.ticker, AggressiveSide, quantity, price};
        return {
            position, buyer.trader->get_id(), seller.trader->get_id(),
            buyer.trader->get_capital(), seller.trader->get_capital()
        };
    }

    double
    potential_match_quantity() const
    {
        return std::min(buyer.quantity, seller.quantity);
    }

    std::optional<util::decimal_price> potential_match_price() const;
};
} // namespace nutc::matching
