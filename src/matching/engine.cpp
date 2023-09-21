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

const std::optional<const std::vector<Match>>
Engine::add_order_and_match(MarketOrder aggressive_order)
{
    if (aggressive_order.side == messages::BUY) {
        std::vector<Match> matches = match_buy_order(aggressive_order);
        const std::optional<const std::vector<Match>> res = matches;
        return (matches.size() > 0) ? res : std::nullopt;
    }
    else {
        std::vector<Match> matches = match_sell_order(aggressive_order);
        const std::optional<const std::vector<Match>> res = matches;
        return (matches.size() > 0) ? res : std::nullopt;
    }
}

std::vector<Match>
Engine::match_sell_order(MarketOrder aggressive_order)
{
    // Assuming incoming is type BUY
    std::vector<Match> matches;
    if (bids.size() == 0 || !bids.top().can_match(aggressive_order)) {
        add_order(aggressive_order);
        return matches;
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
        passive_order.quantity -= quantity_to_match;
        aggressive_order.quantity -= quantity_to_match;
        if (passive_order.quantity > 0) {
            bids.push(passive_order);
            return matches;
        }
        else if (aggressive_order.quantity <= 0) {
            return matches;
        }
    }
    if (aggressive_order.quantity > 0) {
        add_order(aggressive_order);
    }
    return matches;
}

std::vector<Match>
Engine::match_buy_order(MarketOrder aggressive_order)
{
    // Assuming incoming is type BUY
    std::vector<Match> matches;
    if (asks.size() == 0 || !asks.top().can_match(aggressive_order)) {
        add_order(aggressive_order);
        return matches;
    }
    while (asks.size() > 0 && asks.top().can_match(aggressive_order)) {
        MarketOrder passive_order = asks.top();
        asks.pop();
        float quantity_to_match =
            std::min(passive_order.quantity, aggressive_order.quantity);
        float price_to_match = passive_order.price;
        matches.push_back(Match{
            passive_order.ticker, aggressive_order.client_uid, passive_order.client_uid,
            price_to_match, quantity_to_match
        });
        passive_order.quantity -= quantity_to_match;
        aggressive_order.quantity -= quantity_to_match;
        if (passive_order.quantity > 0) {
            asks.push(passive_order);
            return matches;
        }
        else if (aggressive_order.quantity <= 0) {
            return matches;
        }
    }
    if (aggressive_order.quantity > 0) {
        add_order(aggressive_order);
    }
    return matches;
}

} // namespace matching
} // namespace nutc
