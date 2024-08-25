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

    tagged_limit_order&
    add_order(const tagged_limit_order& order) override
    {
        modify_level_(order.side, order.quantity, order.price);

        return BaseOrderBookT::add_order(order);
    }

    void
    mark_order_removed(tagged_limit_order& order) override
    {
        modify_level_(order.side, -order.quantity, order.price);

        BaseOrderBookT::mark_order_removed(order);
    }

    void
    change_quantity(tagged_limit_order& order, double quantity_delta) override
    {
        modify_level_(order.side, quantity_delta, order.price);

        BaseOrderBookT::change_quantity(order, quantity_delta);
    }

private:
    void
    modify_level_(util::Side side, double quantity_delta, util::decimal_price price)
    {
        level_update_generator_.record_level_change(side, quantity_delta, price);
    }
};
} // namespace nutc::matching
