#pragma once

#include "exchange/orders/level_tracking/level_update_generator.hpp"
#include "exchange/orders/storage/order_storage.hpp"
#include "shared/types/decimal_price.hpp"

namespace nutc::matching {

template <class BaseOrderBookT>
class LevelTrackedOrderbook : public BaseOrderBookT {
    LevelUpdateGenerator level_update_generator_;

public:
    LevelUpdateGenerator&
    get_update_generator()
    {
        return level_update_generator_;
    }

    stored_order&
    add_order(const stored_order& order) override
    {
        if (!order.ioc) {
            modify_level_(
                order.position.side, order.position.price, order.position.quantity
            );
        }

        return BaseOrderBookT::add_order(order);
    }

    void
    mark_order_removed(stored_order& order) override
    {
        modify_level_(
            order.position.side, order.position.price, -order.position.quantity
        );

        BaseOrderBookT::mark_order_removed(order);
    }

    void
    change_quantity(stored_order& order, double quantity_delta) override
    {
        modify_level_(order.position.side, order.position.price, quantity_delta);

        BaseOrderBookT::change_quantity(order, quantity_delta);
    }

private:
    void
    modify_level_(util::Side side, util::decimal_price price, double delta)
    {
        level_update_generator_.record_level_change(side, price, delta);
    }
};
} // namespace nutc::matching
