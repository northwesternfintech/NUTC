//
// Created by echav on 9/4/2023.
//

#include "engine.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace nutc {
namespace matching {

Engine::Engine()
{
    // pass
}

void
Engine::add_order_without_matching(MarketOrder aggressive_order)
{
    if (aggressive_order.side == messages::BUY) {
        bids.push(aggressive_order);
    }
    else {
        asks.push(aggressive_order);
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
Engine::get_passive_orders(messages::SIDE side)
{
    return side == messages::SIDE::BUY ? this->asks : this->bids;
}

bool
insufficient_capital(const MarketOrder& order, const manager::ClientManager& manager)
{
    float order_value = order.price * order.quantity;
    float capital = manager.getCapital(order.client_uid);
    return order.side == messages::SIDE::BUY && order_value > capital;
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
Engine::match_order(
    MarketOrder& aggressive_order, const manager::ClientManager& manager
)
{
    MatchResult result;

    if (insufficient_capital(aggressive_order, manager)) {
        return result;
    }

    auto& passive_orders = get_passive_orders(aggressive_order.side);

    if (cannot_match_passive(aggressive_order, passive_orders)) {
        add_order_without_matching(aggressive_order);
        add_ob_update(result.ob_updates, aggressive_order, aggressive_order.quantity);
        return result;
    }

    MatchResult res = attempt_matches(passive_orders, aggressive_order, manager);

    if (aggressive_order.quantity > 0) {
        add_order_without_matching(aggressive_order);
        add_ob_update(res.ob_updates, aggressive_order, aggressive_order.quantity);
    }

    return res;
}

float
Engine::getMatchQuantity(
    const MarketOrder& passive_order, const MarketOrder& aggressive_order
)
{
    return std::min(passive_order.quantity, aggressive_order.quantity);
}

MatchResult
Engine::attempt_matches(
    std::priority_queue<MarketOrder>& passive_orders, MarketOrder& aggressive_order,
    const manager::ClientManager& manager
)
{
    MatchResult result;
    while (passive_orders.size() > 0 && passive_orders.top().can_match(aggressive_order)
    ) {
        MarketOrder passive_order = passive_orders.top();
        float quantity_to_match = getMatchQuantity(passive_order, aggressive_order);

        float price_to_match = passive_order.price;

        Match toMatch = Match{passive_order.ticker,
                              passive_order.client_uid,
                              aggressive_order.client_uid,
                              aggressive_order.side,
                              price_to_match,
                              quantity_to_match};

        std::optional<messages::SIDE> match_failure = manager.validateMatch(toMatch);
        if (match_failure.has_value()) {
            bool aggressive_failure = match_failure.value() == aggressive_order.side;
            if (aggressive_failure) {
                return result;
            }
            passive_orders.pop();
            continue;
        }
        passive_orders.pop();

        add_ob_update(result.ob_updates, passive_order, 0);
        result.matches.push_back(toMatch);
        passive_order.quantity -= quantity_to_match;
        aggressive_order.quantity -= quantity_to_match;
        if (passive_order.quantity > 0) {
            passive_orders.push(passive_order);
            add_ob_update(result.ob_updates, passive_order, passive_order.quantity);
            return result;
        }
        else if (aggressive_order.quantity <= 0) {
            return result;
        }
    }
    return result;
}

} // namespace matching
} // namespace nutc
