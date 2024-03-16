#include "engine.hpp"

#include "exchange/tickers/engine/order_storage.hpp"
#include "exchange/utils/logger/logger.hpp"

#include <algorithm>
#include <vector>

namespace nutc {
namespace matching {

// TODO(anyone): unit test for added orders
on_tick_result_t
Engine::on_tick(uint64_t new_tick, uint8_t order_expire_age)
{
    current_tick_ = new_tick;
    uint64_t removed_tick_age = new_tick - order_expire_age;

    while (!orders_by_tick_.empty()) {
        uint64_t earliest_tick = orders_by_tick_.begin()->first;
        if (earliest_tick > removed_tick_age) {
            break;
        }
        std::queue<uint64_t>& order_ids = orders_by_tick_.at(earliest_tick);
        while (!order_ids.empty()) {
            uint64_t order_id = order_ids.front();
            order_ids.pop();

            if (orders_by_id_.find(order_id) == orders_by_id_.end()) {
                continue;
            }

            double price = orders_by_id_.at(order_id).price;
            SIDE side = orders_by_id_.at(order_id).side;
            if (side == SIDE::BUY)
                bids_.erase(order_index{price, order_id});
            else
                asks_.erase(order_index{price, order_id});
            removed_orders_.push_back(std::move(orders_by_id_.at(order_id)));
            orders_by_id_.erase(order_id);
        }
        orders_by_tick_.erase(earliest_tick);
    }

    std::vector<StoredOrder> return_vec = std::move(removed_orders_);
    removed_orders_.clear();

    std::vector<StoredOrder> added_orders = std::move(added_orders_);
    added_orders_.clear();

    std::vector<Match> matched_orders = std::move(matched_orders_);
    matched_orders_.clear();

    return {return_vec, added_orders, matched_orders};
}

void
add_ob_update(std::vector<ObUpdate>& vec, StoredOrder& order, double quantity)
{
    ObUpdate update{order.ticker, order.side, order.price, quantity};
    if (order.trader->get_type() != manager::BOT) {
        events::Logger& logger = events::Logger::get_logger();
        logger.log_event(update);
    }
    vec.push_back(std::move(update));
}

bool
insufficient_capital(const StoredOrder& order)
{
    double capital = order.trader->get_capital();
    double order_value = order.price * order.quantity;
    return order.side == SIDE::BUY && order_value > capital;
}

bool
insufficient_holdings(const StoredOrder& order)
{
    double holdings = order.trader->get_holdings(order.ticker);
    return order.side == SIDE::SELL && order.quantity > holdings;
}

match_result_t
Engine::match_order(MarketOrder&& order, manager::ClientManager& manager)
{
    match_result_t result;
    StoredOrder stored_order(std::move(order), current_tick_);

    if (insufficient_capital(stored_order)) {
        removed_orders_.push_back(stored_order);
        return result;
    }

    if (insufficient_holdings(stored_order)) {
        removed_orders_.push_back(stored_order);
        return result;
    }

    add_order(stored_order);

    match_result_t res = attempt_matches_(manager, stored_order);

    return res;
}

constexpr bool
is_close_to_zero(double value, double epsilon = std::numeric_limits<double>::epsilon())
{
    return std::fabs(value) < epsilon;
}

constexpr bool
is_same_value(
    double value1, double value2,
    double epsilon = std::numeric_limits<double>::epsilon()
)
{
    return std::fabs(value1 - value2) < epsilon;
}

double
get_match_quantity(
    const StoredOrder& passive_order, const StoredOrder& aggressive_order
)
{
    return std::min(passive_order.quantity, aggressive_order.quantity);
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
    double aggressive_quantity = aggressive_order.quantity;
    uint64_t aggressive_index = aggressive_order.order_index;

    events::Logger& logger = events::Logger::get_logger();

    while (can_match_orders_()) {
        StoredOrder& sell_order_ref =
            get_top_order_(SIDE::SELL).value().get(); // NOLINT(*)
        StoredOrder& buy_order_ref =
            get_top_order_(SIDE::BUY).value().get(); // NOLINT(*)

        double quantity_to_match = get_match_quantity(buy_order_ref, sell_order_ref);
        SIDE aggressive_side = get_aggressive_side(sell_order_ref, buy_order_ref);

        double price_to_match =
            aggressive_side == SIDE::BUY ? sell_order_ref.price : buy_order_ref.price;

        std::string buyer_id = buy_order_ref.trader->get_id();
        std::string seller_id = sell_order_ref.trader->get_id();

        Match to_match{sell_order_ref.ticker, aggressive_side, price_to_match,
                       quantity_to_match,     buyer_id,        seller_id};

        std::optional<SIDE> match_failure = manager.validate_match(to_match);
        if (match_failure.has_value()) {
            SIDE side = match_failure.value();
            if (side == SIDE::BUY) {
                bids_.erase(bids_.begin());
                orders_by_id_.erase(buy_order_ref.order_index);
                removed_orders_.push_back(buy_order_ref);
            }
            else {
                asks_.erase(asks_.begin());
                orders_by_id_.erase(sell_order_ref.order_index);
                removed_orders_.push_back(sell_order_ref);
            }
            continue;
        }

        // Now that we know the match is valid, we can make copies of the order and
        // delete them from the tables
        // This could be optimized, but it's good for now
        StoredOrder sell_order = sell_order_ref;
        StoredOrder buy_order = buy_order_ref;

        removed_orders_.push_back(sell_order);
        removed_orders_.push_back(buy_order);

        matched_orders_.push_back(Match{
            "", aggressive_side, price_to_match, quantity_to_match, buyer_id, seller_id
        });

        orders_by_id_.erase(buy_order_ref.order_index);
        orders_by_id_.erase(sell_order_ref.order_index);
        bids_.erase(bids_.begin());
        asks_.erase(asks_.begin());

        buy_order.quantity -= quantity_to_match;
        sell_order.quantity -= quantity_to_match;

        if (sell_order.trader->get_type() != manager::BOT
            || buy_order.trader->get_type() != manager::BOT) {
            logger.log_event(to_match);
        }
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
            added_orders_.push_back(buy_order);
        }

        if (!is_close_to_zero(sell_order.quantity)) {
            if (!sell_aggressive)
                add_ob_update(result.ob_updates, sell_order, sell_order.quantity);
            add_order(sell_order);
            added_orders_.push_back(sell_order);
        }

        auto update_traders = [&](const std::shared_ptr<manager::GenericTrader>& trader,
                                  SIDE side) {
            if (side == SIDE::BUY) {
                trader->modify_capital(-quantity_to_match * price_to_match);
                trader->modify_holdings(buy_order.ticker, quantity_to_match);
            }
            else {
                trader->modify_capital(quantity_to_match * price_to_match);
                trader->modify_holdings(buy_order.ticker, -quantity_to_match);
            }
        };

        update_traders(buy_order.trader, SIDE::BUY);
        update_traders(sell_order.trader, SIDE::SELL);
    }

    if (aggressive_quantity > 0) {
        add_ob_update(result.ob_updates, aggressive_order, aggressive_quantity);
    }

    return result;
}

} // namespace matching
} // namespace nutc
