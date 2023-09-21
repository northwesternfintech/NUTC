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

std::optional<std::pair<const std::vector<Match>, const std::vector<ObUpdate>>>
Engine::add_order_and_match(MarketOrder aggressive_order)
{
    if (aggressive_order.side == messages::BUY) {
        std::pair<std::vector<Match>, std::vector<ObUpdate>> matches =
            match_buy_order(aggressive_order);
        std::optional<std::pair<const std::vector<Match>, const std::vector<ObUpdate>>>
            res = matches;
        return (matches.first.size() > 0) ? res : std::nullopt;
    }
    else {
        std::pair<std::vector<Match>, std::vector<ObUpdate>> matches =
            match_sell_order(aggressive_order);
        std::optional<std::pair<const std::vector<Match>, const std::vector<ObUpdate>>>
            res = matches;
        return (matches.first.size() > 0) ? res : std::nullopt;
    }
}

ObUpdate
Engine::create_ob_update(const MarketOrder& order, float quantity)
{
    return ObUpdate{order.ticker, order.side, order.price, quantity};
}

std::pair<std::vector<Match>, std::vector<ObUpdate>>
Engine::match_sell_order(MarketOrder aggressive_order)
{
    // Assuming incoming is type BUY
    std::vector<Match> matches;
    std::vector<ObUpdate> ob_updates;
    if (bids.size() == 0 || !bids.top().can_match(aggressive_order)) {
        add_order(aggressive_order);
        ob_updates.push_back(
            create_ob_update(aggressive_order, aggressive_order.quantity)
        );
        return std::make_tuple(matches, ob_updates);
    }
    while (bids.size() > 0 && bids.top().can_match(aggressive_order)) {
        MarketOrder passive_order = bids.top();
        bids.pop();
        float quantity_to_match =
            std::min(passive_order.quantity, aggressive_order.quantity);
        float price_to_match = passive_order.price;
        matches.push_back(Match{
            passive_order.ticker, passive_order.client_uid, aggressive_order.client_uid,
            price_to_match, quantity_to_match
        });
        ob_updates.push_back(create_ob_update(passive_order, 0));
        passive_order.quantity -= quantity_to_match;
        aggressive_order.quantity -= quantity_to_match;
        if (passive_order.quantity > 0) {
            bids.push(passive_order);
            ob_updates.push_back(create_ob_update(passive_order, passive_order.quantity)
            );
            return std::make_tuple(matches, ob_updates);
        }
        else if (aggressive_order.quantity <= 0) {
            return std::make_tuple(matches, ob_updates);
        }
    }
    if (aggressive_order.quantity > 0) {
        ob_updates.push_back(
            create_ob_update(aggressive_order, aggressive_order.quantity)
        );
        add_order(aggressive_order);
    }
    return std::make_tuple(matches, ob_updates);
}

std::pair<std::vector<Match>, std::vector<ObUpdate>>
Engine::match_buy_order(MarketOrder aggressive_order)
{
    // Assuming incoming is type BUY
    std::vector<Match> matches;
    std::vector<ObUpdate> ob_updates;
    if (asks.size() == 0 || !asks.top().can_match(aggressive_order)) {
        add_order(aggressive_order);
        ob_updates.push_back(
            create_ob_update(aggressive_order, aggressive_order.quantity)
        );
        return std::make_tuple(matches, ob_updates);
    }
    while (asks.size() > 0 && asks.top().can_match(aggressive_order)) {
        MarketOrder passive_order = asks.top();
        asks.pop();
        float quantity_to_match =
            std::min(passive_order.quantity, aggressive_order.quantity);
        float price_to_match = passive_order.price;
        matches.push_back(Match{
            passive_order.ticker, passive_order.client_uid, aggressive_order.client_uid,
            price_to_match, quantity_to_match
        });
        ob_updates.push_back(create_ob_update(passive_order, 0));
        passive_order.quantity -= quantity_to_match;
        aggressive_order.quantity -= quantity_to_match;
        if (passive_order.quantity > 0) {
            asks.push(passive_order);
            ob_updates.push_back(create_ob_update(passive_order, passive_order.quantity)
            );
            return std::make_tuple(matches, ob_updates);
        }
        else if (aggressive_order.quantity <= 0) {
            return std::make_tuple(matches, ob_updates);
        }
    }
    if (aggressive_order.quantity > 0) {
        ob_updates.push_back(
            create_ob_update(aggressive_order, aggressive_order.quantity)
        );
        add_order(aggressive_order);
    }
    return std::make_tuple(matches, ob_updates);
}

} // namespace matching
} // namespace nutc
