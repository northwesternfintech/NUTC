#pragma once

#include "common/types/ticker.hpp"
#include "common/util.hpp"
#include "exchange/orders/level_tracking/level_update_generator.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/orders/orderbook/order_id_tracker.hpp"

namespace nutc::exchange {

class CompositeOrderBook {
    using order_list = std::list<tagged_limit_order>;
    LimitOrderBook orderbook_;
    LevelUpdateGenerator level_update_generator_;
    OrderIdTracker order_id_tracker_;

public:
    explicit CompositeOrderBook(common::Ticker ticker) : level_update_generator_(ticker)
    {}

    common::decimal_price
    get_midprice() const
    {
        return orderbook_.get_midprice();
    }

    std::vector<common::position>
    get_and_reset_updates()
    {
        auto updates = level_update_generator_.get_updates();
        level_update_generator_.reset();
        return updates;
    }

    order_list::iterator add_order(const tagged_limit_order& order);

    void remove_order(common::order_id_t order_id);

    void remove_order(order_list::iterator order);

    void change_quantity(
        order_list::iterator order, common::decimal_quantity quantity_delta
    );

    std::optional<LimitOrderBook::stored_limit_order>
    get_top_order(common::Side side)
    {
        return orderbook_.get_top_order(side);
    }
};
} // namespace nutc::exchange
