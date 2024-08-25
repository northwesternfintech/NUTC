#include "order_pair.hpp"

#include "exchange/orders/storage/order_storage.hpp"

namespace nutc::matching {
template <TaggedOrder BuyerT, TaggedOrder SellerT>
requires HasLimitOrder<BuyerT, SellerT>
std::optional<util::decimal_price>
OrderPair<BuyerT, SellerT>::potential_match_price() const
{
    if constexpr (is_market_order_v<BuyerT>)
        return seller.price;
    else if constexpr (is_market_order_v<SellerT>)
        return buyer.price;
    else if (buyer.price < seller.price)
        return std::nullopt;
    else
        return buyer.timestamp < seller.timestamp ? buyer.price : seller.price;
}

template <TaggedOrder BuyerT, TaggedOrder SellerT>
requires HasLimitOrder<BuyerT, SellerT>
void
OrderPair<BuyerT, SellerT>::handle_match(
    const messages::match& match, util::decimal_price order_fee,
    LimitOrderBook& orderbook
)
{
    // Incorrect in presence of level tracker - need to know which is aggressive
    if constexpr (is_limit_order_v<SellerT>) {
        orderbook.change_quantity(seller, -match.position.quantity);
    }
    else {
        seller.active = false;
    }
    if constexpr (is_limit_order_v<BuyerT>) {
        orderbook.change_quantity(buyer, -match.position.quantity);
    }
    else {
        buyer.active = false;
    }
    buyer.trader->notify_match(
        {match.position.ticker, util::Side::buy, match.position.quantity,
         match.position.price * (util::decimal_price{1.0} + order_fee)}
    );
    seller.trader->notify_match(
        {match.position.ticker, util::Side::sell, match.position.quantity,
         match.position.price * (util::decimal_price{1.0} - order_fee)}
    );
}

template class OrderPair<tagged_limit_order, tagged_limit_order>;
template class OrderPair<tagged_limit_order, tagged_market_order>;
template class OrderPair<tagged_market_order, tagged_limit_order>;
} // namespace nutc::matching
