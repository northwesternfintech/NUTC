#include "engine.hpp"

#include "utils/logger/logger.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace nutc {
namespace matching {

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
Engine::get_orders(SIDE side)
{
    return side == SIDE::SELL ? this->asks : this->bids;
}

bool
Engine::insufficient_capital(
    const MarketOrder& order, const manager::ClientManager& manager
)
{
    float capital = manager.get_capital(order.client_uid);
    float order_value = order.price * order.quantity;
    return order.side == SIDE::BUY && order_value > capital;
}

bool
insufficient_holdings(const MarketOrder& order, const manager::ClientManager& manager)
{
    float holdings = manager.get_holdings(order.client_uid, order.ticker);
    return order.side == SIDE::SELL && order.quantity > holdings;
}

MatchResult
Engine::match_order(MarketOrder& order, manager::ClientManager& manager)
{
    MatchResult result;

    if (insufficient_capital(order, manager)) {
        return result;
    }

    if (insufficient_holdings(order, manager)) {
        return result;
    }

    get_orders(order.side).push(order);

    MatchResult res = attempt_matches(manager, order);

    events::Logger& logger = events::Logger::get_logger();

    // Log information from res
    for (const auto& match : res.matches) {
        logger.log_event(match);
    }

    for (const auto& ob_update : res.ob_updates) {
        logger.log_event(ob_update);
    }

    return res;
}

inline constexpr bool
is_close_to_zero(float value, float epsilon = 1e-6f)
{
    return std::fabs(value) < epsilon;
}

inline constexpr bool
is_same_value(float value1, float value2, float epsilon = 1e-6f)
{
    return std::fabs(value1 - value2) < epsilon;
}

float
Engine::get_match_quantity(
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

SIDE
Engine::get_aggressive_side(const MarketOrder& order1, const MarketOrder& order2)
{
    return order1.order_index > order2.order_index ? order1.side : order2.side;
}

MatchResult
Engine::attempt_matches(
    manager::ClientManager& manager, const MarketOrder& aggressive_order
)
{
    MatchResult result;
    float aggressive_quantity = aggressive_order.quantity;
    float aggressive_index = aggressive_order.order_index;

    while (bids.size() > 0 && asks.size() > 0 && bids.top().can_match(asks.top())) {
        MarketOrder sell_order = asks.top();
        MarketOrder buy_order = bids.top();

        float quantity_to_match = get_match_quantity(buy_order, sell_order);
        SIDE aggressive_side = get_aggressive_side(sell_order, buy_order);

        float price_to_match =
            aggressive_side == SIDE::BUY ? sell_order.price : buy_order.price;

        std::string buyer_uid = buy_order.client_uid;
        std::string seller_uid = sell_order.client_uid;

        Match toMatch = Match{sell_order.ticker, buyer_uid,      seller_uid,
                              aggressive_side,   price_to_match, quantity_to_match};

        std::optional<SIDE> match_failure = manager.validate_match(toMatch);
        if (match_failure.has_value()) {
            SIDE side = match_failure.value();
            if (side == SIDE::BUY)
                bids.pop();
            else
                asks.pop();
            continue;
        }

        last_sell_price = price_to_match;

        bids.pop();
        asks.pop();

        buy_order.quantity -= quantity_to_match;
        sell_order.quantity -= quantity_to_match;

        events::Logger& logger = events::Logger::get_logger();
        logger.log_event(toMatch);

        result.matches.push_back(toMatch);

        bool sell_aggressive = is_same_value(sell_order.order_index, aggressive_index);
        bool buy_aggressive = is_same_value(buy_order.order_index, aggressive_index);

        if (buy_aggressive)
            aggressive_quantity -= quantity_to_match;
        else
            add_ob_update(result.ob_updates, buy_order, 0);

        if (sell_aggressive)
            aggressive_quantity -= quantity_to_match;
        else
            add_ob_update(result.ob_updates, sell_order, 0);

        if (!is_close_to_zero(buy_order.quantity)) {
            if (!buy_aggressive)
                add_ob_update(result.ob_updates, buy_order, buy_order.quantity);
            bids.push(buy_order);
        }

        if (!is_close_to_zero(sell_order.quantity)) {
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
