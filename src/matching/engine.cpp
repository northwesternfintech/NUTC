#include "engine.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace nutc {
namespace matching {

Engine::Engine() {}

void
Engine::add_order_without_matching(MarketOrder order)
{
    if (order.side == messages::BUY) {
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
Engine::get_passive_orders(messages::SIDE side)
{
    return side == messages::SIDE::BUY ? this->asks : this->bids;
}

bool
insufficient_capital(
    const MarketOrder& aggressive_order, float order_value,
    const manager::ClientManager& manager
)
{
    float capital = manager.getCapital(aggressive_order.client_uid);
    return aggressive_order.side == messages::SIDE::BUY && order_value > capital;
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

    auto& passive_orders = get_passive_orders(order.side);

    MatchResult res = attempt_matches(passive_orders, order, manager);

    return res;
}

bool
isCloseToZero(float value, float epsilon = 1e-6f)
{
    return std::fabs(value) < epsilon;
}

float
Engine::getMatchQuantity(
    const MarketOrder& passive_order, const MarketOrder& aggressive_order
)
{
    return std::min(passive_order.quantity, aggressive_order.quantity);
}

// TODO: modify so it's not matching an incoming and passive order, it just matches
// orders from both sides
MatchResult
Engine::attempt_matches(
    std::priority_queue<MarketOrder>& passive_orders, MarketOrder& aggressive_order,
    manager::ClientManager& manager
)
{
    MatchResult result;

    while (passive_orders.size() > 0 && passive_orders.top().can_match(aggressive_order)
    ) {
        MarketOrder passive_order = passive_orders.top();
        float quantity_to_match = getMatchQuantity(passive_order, aggressive_order);
        float price_to_match = passive_order.price;
        std::string buyer_uid = passive_order.side == messages::SIDE::BUY
                                    ? passive_order.client_uid
                                    : aggressive_order.client_uid;
        std::string seller_uid = passive_order.side == messages::SIDE::SELL
                                     ? passive_order.client_uid
                                     : aggressive_order.client_uid;

        Match toMatch = Match{passive_order.ticker,  buyer_uid,      seller_uid,
                              aggressive_order.side, price_to_match, quantity_to_match};

        std::optional<messages::SIDE> match_failure = manager.validateMatch(toMatch);
        if (match_failure.has_value()) {
            bool aggressive_failure = match_failure.value() == aggressive_order.side;
            if (aggressive_failure) {
                return result;
            }
            else {
                passive_orders.pop();
                continue;
            }
        }
        passive_orders.pop();

        add_ob_update(result.ob_updates, passive_order, 0);
        result.matches.push_back(toMatch);
        passive_order.quantity -= quantity_to_match;
        aggressive_order.quantity -= quantity_to_match;
        if (passive_order.side == messages::SIDE::SELL) {
            manager.modifyCapital(
                passive_order.client_uid, quantity_to_match * price_to_match
            );
            manager.modifyCapital(
                aggressive_order.client_uid, -quantity_to_match * price_to_match
            );
            manager.modifyHoldings(
                aggressive_order.client_uid, aggressive_order.ticker, quantity_to_match
            );
            manager.modifyHoldings(
                passive_order.client_uid, passive_order.ticker, -quantity_to_match
            );
        }
        else {
            manager.modifyCapital(
                passive_order.client_uid, -quantity_to_match * price_to_match
            );
            manager.modifyCapital(
                aggressive_order.client_uid, quantity_to_match * price_to_match
            );
            manager.modifyHoldings(
                aggressive_order.client_uid, aggressive_order.ticker, -quantity_to_match
            );
            manager.modifyHoldings(
                passive_order.client_uid, passive_order.ticker, quantity_to_match
            );
        }
        if (!isCloseToZero(passive_order.quantity)) {
            passive_orders.push(passive_order);
            add_ob_update(result.ob_updates, passive_order, passive_order.quantity);
        }
        // cannot match anymore
        if (isCloseToZero(aggressive_order.quantity)) {
            return result;
        }
    }
    if (!isCloseToZero(aggressive_order.quantity)) {
        add_order_without_matching(aggressive_order);
        add_ob_update(result.ob_updates, aggressive_order, aggressive_order.quantity);
    }
    return result;
}

} // namespace matching
} // namespace nutc
