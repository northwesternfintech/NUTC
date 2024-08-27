#pragma once

#include "common/types/decimal.hpp"
#include "common/types/ticker.hpp"
#include "exchange/orders/level_tracking/level_update_generator.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/orders/storage/order_storage.hpp"

namespace nutc::exchange {

template <class BaseOrderBookT>
class LevelTrackedOrderbook : public BaseOrderBookT {
    LevelUpdateGenerator level_update_generator_;

public:
    explicit LevelTrackedOrderbook(common::Ticker ticker) :
        level_update_generator_{ticker}
    {}

    LevelUpdateGenerator&
    get_update_generator()
    {
        return level_update_generator_;
    }

    LimitOrderBook::stored_limit_order
    add_order(const tagged_limit_order& order) override
    {
        modify_level_(order.side, order.quantity, order.price);

        return BaseOrderBookT::add_order(order);
    }

    void
    mark_order_removed(LimitOrderBook::stored_limit_order order) override
    {
        modify_level_(order->side, -order->quantity, order->price);

        BaseOrderBookT::mark_order_removed(order);
    }

    void
    change_quantity(
        LimitOrderBook::stored_limit_order order,
        common::decimal_quantity quantity_delta
    ) override
    {
        modify_level_(order->side, quantity_delta, order->price);

        BaseOrderBookT::change_quantity(order, quantity_delta);
    }

private:
    void
    modify_level_(
        common::Side side, common::decimal_quantity quantity_delta,
        common::decimal_price price
    )
    {
        level_update_generator_.record_level_change(side, quantity_delta, price);
    }
};
} // namespace nutc::exchange
