#include "engine.hpp"

#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/util.hpp"

namespace nutc {
namespace matching {

using match = nutc::messages::match;

template <util::Side AggressiveSide>
std::expected<stored_match, bool>
Engine::match_orders(
    tagged_limit_order& buyer, tagged_limit_order& seller, LimitOrderBook& orderbook
)
{
    auto match_result = attempt_match<AggressiveSide>(buyer, seller);
    if (match_result.has_value()) {
        orderbook.change_quantity(seller, -match_result->position.quantity);
        orderbook.change_quantity(buyer, -match_result->position.quantity);
        return match_result.value();
    }
    else if (match_result.error() == seller_failure) {
        orderbook.mark_order_removed(seller);
        if constexpr (AggressiveSide == side::buy) return std::unexpected(false);
    }
    else if (match_result.error() == buyer_failure) {
        orderbook.mark_order_removed(buyer);
        if constexpr (AggressiveSide == side::sell) return std::unexpected(false);
    }
    return std::unexpected(true);
}

template <util::Side AggressiveSide>
std::expected<stored_match, bool>
Engine::match_incoming_order(
    tagged_limit_order& aggressive_order, tagged_limit_order& passive_order,
    LimitOrderBook& orderbook
)
{
    if constexpr (AggressiveSide == side::buy) {
        return match_orders<AggressiveSide>(aggressive_order, passive_order, orderbook);
    }
    else {
        return match_orders<AggressiveSide>(passive_order, aggressive_order, orderbook);
    }
}

template <util::Side AggressiveSide>
auto
Engine::attempt_match(tagged_limit_order& buyer, tagged_limit_order& seller)
    -> std::expected<stored_match, no_match_reason>
{
    if (buyer.price < seller.price) return std::unexpected(done_matching);
    decimal_price match_price;
    if constexpr (AggressiveSide == side::buy) {
        match_price = seller.price;
    }
    else {
        match_price = buyer.price;
    }
    double match_quantity = order_quantity(buyer, seller);
    decimal_price total_price{calculate_order_cost(match_price, match_quantity)};
    if (buyer.trader->get_capital() < total_price)
        return std::unexpected(buyer_failure);
    if (seller.trader->get_holdings(seller.ticker) < match_quantity)
        return std::unexpected(seller_failure);
    if (buyer.trader == seller.trader) {
        return std::unexpected(buyer_failure);
    }
    return create_match(match_price, buyer, seller);
}

std::vector<stored_match>
Engine::match_order(const tagged_limit_order& incoming_order, LimitOrderBook& orderbook)
{
    std::vector<stored_match> matches;

    tagged_limit_order order = incoming_order;

    auto match_orderbook = [&](auto& aggressive_order,
                               auto& orderbook) -> std::expected<stored_match, bool> {
        if (aggressive_order.side == side::buy) {
            auto passive_order = orderbook.get_top_order(side::sell);
            if (!passive_order.has_value()) return std::unexpected(true);
            return match_incoming_order<side::buy>(
                aggressive_order, *passive_order, orderbook
            );
        }
        else {
            auto passive_order = orderbook.get_top_order(side::buy);
            if (!passive_order.has_value()) return std::unexpected(true);
            return match_incoming_order<side::sell>(
                aggressive_order, *passive_order, orderbook
            );
        }
    };

    while (order.active) {
        auto match_opt = match_orderbook(order, orderbook);
        if (match_opt.has_value())
            matches.push_back(match_opt.value());
        else if (match_opt.error())
            break;
    }

    if (!order.ioc && !util::is_close_to_zero(order.quantity)) {
        orderbook.add_order(order);
    }

    return matches;
}

stored_match
Engine::create_match(
    util::decimal_price price, tagged_limit_order& buyer, tagged_limit_order& seller
)
{
    double quantity = order_quantity(buyer, seller);

    util::Side aggressive_side =
        buyer.timestamp < seller.timestamp ? util::Side::sell : util::Side::buy;

    auto& ticker = buyer.ticker;
    buyer.trader->process_order_match(
        {ticker, util::Side::buy, quantity,
         price * (util::decimal_price{1.0} + order_fee)}
    );
    seller.trader->process_order_match(
        {ticker, util::Side::sell, quantity,
         price * (util::decimal_price{1.0} - order_fee)}
    );

    util::position position{buyer.ticker, aggressive_side, quantity, price};
    stored_match match{*buyer.trader, *seller.trader, position};
    return match;
}

} // namespace matching
} // namespace nutc
