#include "engine.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace nutc {
namespace matching {

Engine::Engine() {}

float
Engine::get_last_sell_price()
{
    return last_sell_price;
}

void
Engine::add_order_without_matching(MarketOrder order)
{
    if (order.side == SIDE::BUY) {
        bids.push(order);
    }
    else {
        asks.push(order);
    }
}

constexpr ObUpdate
create_ob_update(const MarketOrder& order, float quantity)
{
    return ObUpdate{order.ticker, order.side, order.price, quantity};
}

void
add_ob_update(std::vector<ObUpdate>& vec, const MarketOrder& order, float quantity)
{
    vec.push_back(create_ob_update(order, quantity));
}

std::priority_queue<MarketOrder>&
Engine::get_respective_orders(SIDE side)
{
    return side == SIDE::SELL ? this->asks : this->bids;
}

bool
insufficient_capital(
    const MarketOrder& aggressive_order, float order_value,
    const manager::ClientManager& manager
)
{
    float capital = manager.get_capital(aggressive_order.client_uid);
    return aggressive_order.side == SIDE::BUY && order_value > capital;
}

bool
cannot_match_passive(
    const MarketOrder& aggressive_order,
    std::priority_queue<MarketOrder>& passive_orders
)
{
    return (passive_orders.size() == 0)
           || !(passive_orders.top().can_match(aggressive_order));
}

MatchResult
Engine::match_order(MarketOrder& order, manager::ClientManager& manager)
{
    MatchResult result;

    if (insufficient_capital(order, order.quantity * order.price, manager)) {
        return result;
    }

    // TODO: insufficient holdings

    auto& respective_orders = get_respective_orders(order.side);
    respective_orders.push(order);

    MatchResult res = attempt_matches(manager, order);

    return res;
}

inline constexpr bool
isCloseToZero(float value, float epsilon = 1e-6f)
{
    return std::fabs(value) < epsilon;
}

inline constexpr bool
isSameValue(float value1, float value2, float epsilon = 1e-6f)
{
    return std::fabs(value1 - value2) < epsilon;
}

float
Engine::getMatchQuantity(
    const MarketOrder& passive_order, const MarketOrder& aggressive_order
)
{
    return std::min(passive_order.quantity, aggressive_order.quantity);
}

std::string
Engine::get_client_uid(
    SIDE side, const MarketOrder& aggressive, const MarketOrder& passive
)
{
    return side == aggressive.side ? aggressive.client_uid : passive.client_uid;
}

// TODO: modify so it's not matching an incoming and passive order, it just matches
// orders from both sides
MatchResult
Engine::attempt_matches(
    manager::ClientManager& manager, const MarketOrder& aggressive_order
)
{
    MatchResult result;
    float aggressive_quantity = aggressive_order.quantity;
    float aggressive_order_index = aggressive_order.order_index;

    while (bids.size() > 0 && asks.size() > 0 && bids.top().can_match(asks.top())) {
        MarketOrder sell_order = asks.top();
        MarketOrder buy_order = bids.top();

        float quantity_to_match = getMatchQuantity(buy_order, sell_order);
        SIDE aggressive_side = buy_order.order_index > sell_order.order_index
                                   ? buy_order.side
                                   : sell_order.side;
        float price_to_match =
            aggressive_side == SIDE::BUY ? sell_order.price : buy_order.price;

        std::string buyer_uid = buy_order.client_uid;
        std::string seller_uid = sell_order.client_uid;

        Match toMatch = Match{sell_order.ticker, buyer_uid,      seller_uid,
                              aggressive_side,   price_to_match, quantity_to_match};

        std::optional<SIDE> match_failure = manager.validate_match(toMatch);
        if (match_failure.has_value()) {
            switch (match_failure.value()) {
                case SIDE::BUY:
                    bids.pop();
                    break;
                case SIDE::SELL:
                    asks.pop();
                    break;
            }
            continue;
        }

        last_sell_price = price_to_match;

        bids.pop();
        asks.pop();

        buy_order.quantity -= quantity_to_match;
        sell_order.quantity -= quantity_to_match;

        result.matches.push_back(toMatch);

        bool sell_aggressive =
            isSameValue(sell_order.order_index, aggressive_order_index);
        bool buy_aggressive =
            isSameValue(buy_order.order_index, aggressive_order_index);

        if (buy_aggressive)
            aggressive_quantity -= quantity_to_match;
        else
            add_ob_update(result.ob_updates, buy_order, 0);

        if (sell_aggressive)
            aggressive_quantity -= quantity_to_match;
        else
            add_ob_update(result.ob_updates, sell_order, 0);

        if (!isCloseToZero(buy_order.quantity)) {
            if (!buy_aggressive)
                add_ob_update(result.ob_updates, buy_order, buy_order.quantity);
            bids.push(buy_order);
        }

        if (!isCloseToZero(sell_order.quantity)) {
            if (!sell_aggressive)
                add_ob_update(result.ob_updates, sell_order, sell_order.quantity);
            asks.push(sell_order);
        }

        manager.modify_capital(buyer_uid, -quantity_to_match * price_to_match);
        manager.modify_capital(seller_uid, quantity_to_match * price_to_match);
        manager.modify_holdings(seller_uid, buy_order.ticker, -quantity_to_match);
        manager.modify_holdings(buyer_uid, buy_order.ticker, quantity_to_match);
    }

    if (aggressive_quantity > 0) {
        add_ob_update(result.ob_updates, aggressive_order, aggressive_quantity);
    }

    return result;
}

} // namespace matching
} // namespace nutc
