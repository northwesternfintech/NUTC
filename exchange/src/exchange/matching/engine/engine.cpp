#include "engine.hpp"

#include "exchange/matching/engine/order_storage.hpp"
#include "exchange/utils/logger/logger.hpp"

#include <algorithm>
#include <vector>

namespace nutc {
namespace matching {

void
Engine::add_order_without_matching(const MarketOrder& order)
{
    StoredOrder stored_order{
        order.client_id, order.side, order.ticker, order.quantity, order.price
    };
    if (order.side == SIDE::BUY)
        bids_.push(std::move(stored_order));
    else
        asks_.push(std::move(stored_order));
}

void
add_ob_update(std::vector<ObUpdate>& vec, const StoredOrder& order, float quantity)
{
    vec.push_back(ObUpdate{order.ticker, order.side, order.price, quantity});
}

std::priority_queue<StoredOrder>&
Engine::get_orders_(SIDE side)
{
    return side == SIDE::SELL ? this->asks_ : this->bids_;
}

bool
insufficient_capital(
    const StoredOrder& order, const manager::ClientManager& manager
)
{
    float capital = manager.get_capital(order.client_id);
    float order_value = order.price * order.quantity;
    return order.side == SIDE::BUY && order_value > capital;
}

bool
insufficient_holdings(const StoredOrder& order, const manager::ClientManager& manager)
{
    float holdings = manager.get_holdings(order.client_id, order.ticker);
    return order.side == SIDE::SELL && order.quantity > holdings;
}

match_result_t
Engine::match_order(MarketOrder&& order, manager::ClientManager& manager)
{
    match_result_t result;
    StoredOrder stored_order(std::move(order));
    if (insufficient_capital(stored_order, manager)) {
        return result;
    }

    if (insufficient_holdings(stored_order, manager)) {
        return result;
    }


    get_orders_(order.side).push(stored_order);

    match_result_t res = attempt_matches_(manager, stored_order);

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

constexpr bool
is_close_to_zero(float value, float epsilon = std::numeric_limits<float>::epsilon())
{
    return std::fabs(value) < epsilon;
}

constexpr bool
is_same_value(
    float value1, float value2, float epsilon = std::numeric_limits<float>::epsilon()
)
{
    return std::fabs(value1 - value2) < epsilon;
}

float
get_match_quantity(
    const StoredOrder& passive_order, const StoredOrder& aggressive_order
)
{
    return std::min(passive_order.quantity, aggressive_order.quantity);
}

std::string
get_client_id(
    SIDE side, const StoredOrder& aggressive, const StoredOrder& passive
)
{
    return side == aggressive.side ? aggressive.client_id : passive.client_id;
}

SIDE
get_aggressive_side(const StoredOrder& order1, const StoredOrder& order2)
{
    return order1.order_index > order2.order_index ? order1.side : order2.side;
}

match_result_t
Engine::attempt_matches_( // NOLINT (cognitive-complexity-*)
    manager::ClientManager& manager, const StoredOrder& aggressive_order
)
{
    match_result_t result;
    float aggressive_quantity = aggressive_order.quantity;
    int64_t aggressive_index = aggressive_order.order_index;

    while (!bids_.empty() && !asks_.empty() && bids_.top().can_match(asks_.top())) {
        StoredOrder sell_order = asks_.top();
        StoredOrder buy_order = bids_.top();

        float quantity_to_match = get_match_quantity(buy_order, sell_order);
        SIDE aggressive_side = get_aggressive_side(sell_order, buy_order);

        float price_to_match =
            aggressive_side == SIDE::BUY ? sell_order.price : buy_order.price;

        std::string buyer_id = buy_order.client_id;
        std::string seller_id = sell_order.client_id;

        Match to_match{sell_order.ticker, aggressive_side, price_to_match,
                       quantity_to_match, buyer_id,        seller_id};

        std::optional<SIDE> match_failure = manager.validate_match(to_match);
        if (match_failure.has_value()) {
            SIDE side = match_failure.value();
            if (side == SIDE::BUY)
                bids_.pop();
            else
                asks_.pop();
            continue;
        }

        bids_.pop();
        asks_.pop();

        buy_order.quantity -= quantity_to_match;
        sell_order.quantity -= quantity_to_match;

        events::Logger& logger = events::Logger::get_logger();
        logger.log_event(to_match);

        result.matches.push_back(to_match);

        bool sell_aggressive = sell_order.order_index == aggressive_index;
        bool buy_aggressive = buy_order.order_index == aggressive_index;

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
            bids_.push(buy_order);
        }

        if (!is_close_to_zero(sell_order.quantity)) {
            if (!sell_aggressive)
                add_ob_update(result.ob_updates, sell_order, sell_order.quantity);
            asks_.push(sell_order);
        }

        manager.modify_capital(buyer_id, -quantity_to_match * price_to_match);
        manager.modify_capital(seller_id, quantity_to_match * price_to_match);
        manager.modify_holdings(seller_id, buy_order.ticker, -quantity_to_match);
        manager.modify_holdings(buyer_id, buy_order.ticker, quantity_to_match);
    }

    if (aggressive_quantity > 0) {
        add_ob_update(result.ob_updates, aggressive_order, aggressive_quantity);
    }

    return result;
}

} // namespace matching
} // namespace nutc
