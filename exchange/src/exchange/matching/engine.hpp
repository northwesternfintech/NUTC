#pragma once

#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/types/decimal_price.hpp"

#include <glaze/util/expected.hpp>

#include <expected>

namespace nutc {
namespace matching {

template <typename... Ts>
concept HasLimitOrder = std::disjunction_v<is_limit_order<Ts>...>;

class Engine {
    using decimal_price = util::decimal_price;

    decimal_price order_fee_;
    using side = nutc::util::Side;
    using match = messages::match;

public:
    explicit Engine(decimal_price order_fee = 0.0) : order_fee_(order_fee) {}

    template <TaggedOrder OrderT>
    std::vector<match> match_order(OrderT order, LimitOrderBook& orderbook);

private:
    template <TaggedOrder OrderT>
    glz::expected<match, bool>
    match_incoming_order_(OrderT& aggressive_order, LimitOrderBook& orderbook);

    template <util::Side AggressiveSide, TaggedOrder OrderT>
    glz::expected<match, bool> match_incoming_order_(
        OrderT& aggressive_order, tagged_limit_order& passive_order,
        LimitOrderBook& orderbook
    );

    template <util::Side AggressiveSide, TaggedOrder BuyerT, TaggedOrder SellerT>
    requires HasLimitOrder<BuyerT, SellerT>
    glz::expected<match, bool>
    match_orders_(BuyerT& buyer, SellerT& seller, LimitOrderBook& orderbook);

    enum class MatchFailure { buyer_failure, seller_failure, done_matching };

    template <util::Side AggressiveSide, TaggedOrder BuyerT, TaggedOrder SellerT>
    requires HasLimitOrder<BuyerT, SellerT>
    glz::expected<match, MatchFailure> attempt_match_(BuyerT& buyer, SellerT& seller);

    template <TaggedOrder BuyerT, TaggedOrder SellerT>
    requires HasLimitOrder<BuyerT, SellerT>
    match
    create_match_(decimal_price price, double quantity, BuyerT& buyer, SellerT& seller);

    decimal_price total_order_cost_(decimal_price price, double quantity) const;

    template <TaggedOrder BuyerT, TaggedOrder SellerT>
    requires HasLimitOrder<BuyerT, SellerT>
    static std::optional<util::decimal_price>
    potential_match_price(const BuyerT& buyer, const SellerT& seller);

    template <TaggedOrder BuyerT, TaggedOrder SellerT>
    static double potential_match_quantity(const BuyerT& order1, const SellerT& order2);
};

} // namespace matching
} // namespace nutc
