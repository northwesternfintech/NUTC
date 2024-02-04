#include "engine.hpp"

#include "exchange/matching/engine/order_storage.hpp"
#include "exchange/utils/logger/logger.hpp"

#include <algorithm>
#include <vector>

namespace nutc {
namespace matching {

float
Engine::get_midprice() const
{
    if (asks_.empty() || bids_.empty()) {
        return 0;
    }
    return (asks_.begin()->price + bids_.rbegin()->price) / 2;
}

std::vector<StoredOrder>
Engine::remove_old_orders(uint64_t new_tick, uint64_t removed_tick_age)
{
    current_tick_ = new_tick;
    std::vector<StoredOrder> removed_orders;
    // Maybe we can reserve space?

    while (!orders_by_tick_.empty()) {
        uint64_t earliest_tick = orders_by_tick_.begin()->first;
        if (earliest_tick > removed_tick_age) {
            break;
        }
        std::queue<uint64_t>& order_ids = orders_by_tick_[earliest_tick];
        while (!order_ids.empty()) {
            uint64_t order_id = order_ids.front();
            order_ids.pop();

            if (orders_by_id_.find(order_id) == orders_by_id_.end()) {
                continue;
            }

            removed_orders.push_back(std::move(orders_by_id_[order_id]));
            orders_by_id_.erase(order_id);
        }
        orders_by_tick_.erase(earliest_tick);
    }

    return removed_orders;
}

void
add_ob_update(std::vector<ObUpdate>& vec, StoredOrder& order, float quantity)
{
    vec.push_back(ObUpdate{order.ticker, order.side, order.price, quantity});
}

bool
insufficient_capital(const StoredOrder& order, const manager::ClientManager& manager)
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
    StoredOrder stored_order(std::move(order), current_tick_);
    if (insufficient_capital(stored_order, manager)) {
        return result;
    }

    if (insufficient_holdings(stored_order, manager)) {
        return result;
    }

    add_order(stored_order);

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
get_client_id(SIDE side, const StoredOrder& aggressive, const StoredOrder& passive)
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
    manager::ClientManager& manager, StoredOrder& aggressive_order
)
{
    match_result_t result;
    float aggressive_quantity = aggressive_order.quantity;
    uint64_t aggressive_index = aggressive_order.order_index;

    while (can_match_orders_()) {
        StoredOrder& sell_order_ref =
            get_top_order_(SIDE::SELL).value().get(); // NOLINT(*)
        StoredOrder& buy_order_ref =
            get_top_order_(SIDE::BUY).value().get(); // NOLINT(*)

        float quantity_to_match = get_match_quantity(buy_order_ref, sell_order_ref);
        SIDE aggressive_side = get_aggressive_side(sell_order_ref, buy_order_ref);

        float price_to_match =
            aggressive_side == SIDE::BUY ? sell_order_ref.price : buy_order_ref.price;

        std::string buyer_id = buy_order_ref.client_id;
        std::string seller_id = sell_order_ref.client_id;

        Match to_match{sell_order_ref.ticker, aggressive_side, price_to_match,
                       quantity_to_match,     buyer_id,        seller_id};

        std::optional<SIDE> match_failure = manager.validate_match(to_match);
        if (match_failure.has_value()) {
            SIDE side = match_failure.value();
            if (side == SIDE::BUY) {
                bids_.erase(bids_.begin());
                orders_by_id_.erase(buy_order_ref.order_index);
            }
            else {
                asks_.erase(asks_.begin());
                orders_by_id_.erase(sell_order_ref.order_index);
            }
            continue;
        }

        // Now that we know the match is valid, we can make copies of the order and
        // delete them from the tables
        // This could be optimized, but it's good for now
        StoredOrder sell_order = sell_order_ref;
        StoredOrder buy_order = buy_order_ref;

        orders_by_id_.erase(buy_order_ref.order_index);
        orders_by_id_.erase(sell_order_ref.order_index);
        bids_.erase(bids_.begin());
        asks_.erase(asks_.begin());

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
            add_order(buy_order);
        }

        if (!is_close_to_zero(sell_order.quantity)) {
            if (!sell_aggressive)
                add_ob_update(result.ob_updates, sell_order, sell_order.quantity);
            add_order(sell_order);
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
