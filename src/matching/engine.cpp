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
Engine::add_order(MarketOrder aggressive_order)
{
    if (aggressive_order.side == messages::BUY) {
        bids.push(aggressive_order);
    }
    else {
        asks.push(aggressive_order);
    }
}

ObUpdate
Engine::create_ob_update(const MarketOrder& order, float quantity)
{
    return ObUpdate{order.ticker, order.side, order.price, quantity};
}

std::pair<std::vector<Match>, std::vector<ObUpdate>>
Engine::match_order(MarketOrder aggressive_order)
{
    std::priority_queue<MarketOrder>& passive_orders =
        aggressive_order.side == messages::SIDE::SELL ? this->bids : this->asks;
    // Assuming incoming is type BUY
    std::vector<Match> matches;
    std::vector<ObUpdate> ob_updates;
    if (passive_orders.size() == 0
        || !passive_orders.top().can_match(aggressive_order)) {
        add_order(aggressive_order);
        ob_updates.push_back(
            create_ob_update(aggressive_order, aggressive_order.quantity)
        );
        return std::make_pair(matches, ob_updates);
    }

    while (passive_orders.size() > 0 && passive_orders.top().can_match(aggressive_order)
    ) {
        MarketOrder passive_order = passive_orders.top();
        passive_orders.pop();
        float quantity_to_match =
            std::min(passive_order.quantity, aggressive_order.quantity);
        float price_to_match = passive_order.price;

        matches.push_back(Match{
            passive_order.ticker, passive_order.client_uid, aggressive_order.client_uid,
            aggressive_order.side, price_to_match, quantity_to_match
        });
        ob_updates.push_back(create_ob_update(passive_order, 0));
        passive_order.quantity -= quantity_to_match;
        aggressive_order.quantity -= quantity_to_match;
        if (passive_order.quantity > 0) {
            passive_orders.push(passive_order);
            ob_updates.push_back(create_ob_update(passive_order, passive_order.quantity)
            );
            return std::make_pair(matches, ob_updates);
        }
        else if (aggressive_order.quantity <= 0) {
            return std::make_pair(matches, ob_updates);
        }
    }
    if (aggressive_order.quantity > 0) {
        ob_updates.push_back(
            create_ob_update(aggressive_order, aggressive_order.quantity)
        );
        add_order(aggressive_order);
    }
    return std::make_pair(matches, ob_updates);
}

} // namespace matching
} // namespace nutc
