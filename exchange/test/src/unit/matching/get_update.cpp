#include "common/util.hpp"
#include "config.hpp"
#include "exchange/orders/orderbook/composite_orderbook.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

#include <algorithm>

namespace nutc::test {
using common::Ticker;
using common::Side::buy;
using common::Side::sell;

class UnitGetUpdate : public ::testing::Test {
protected:
    TestTrader trader1{"ABC", TEST_STARTING_CAPITAL};
    TestTrader trader2{"DEF", TEST_STARTING_CAPITAL};
    TestTrader trader3{"GHI", TEST_STARTING_CAPITAL};

    nutc::exchange::CompositeOrderBook orderbook_{Ticker::ETH};
};

TEST_F(UnitGetUpdate, NoOrders)
{
    // auto updates = get_and_reset_updates(Ticker::ETH, before, after);
    auto updates = orderbook_.get_and_reset_updates();

    ASSERT_EQ(updates.size(), 0);
}

TEST_F(UnitGetUpdate, OrderAdded)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};

    orderbook_.add_order(order1);

    auto updates = orderbook_.get_and_reset_updates();

    ASSERT_EQ(updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 1, 1);
}

TEST_F(UnitGetUpdate, OrderDeleted)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};

    // before, we have a single order
    auto o1_it = orderbook_.add_order(order1);

    orderbook_.get_and_reset_updates();

    // we delete the order

    orderbook_.remove_order(o1_it);
    //
    auto updates = orderbook_.get_and_reset_updates();

    ASSERT_EQ(updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 0, 1);
}

TEST_F(UnitGetUpdate, OrderQuantityChange)
{
    double initial_quantity = 1;
    tagged_limit_order order1{trader1, Ticker::ETH, buy, initial_quantity, 1.0};
    auto o1_it = orderbook_.add_order(order1);

    double quantity_delta = 5;
    orderbook_.change_quantity(o1_it, quantity_delta);
    auto updates = orderbook_.get_and_reset_updates();

    ASSERT_EQ(updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(
        updates[0], Ticker::ETH, buy, quantity_delta + initial_quantity, 1
    );
}

// This is an edge case that we currently level change updator doesn't handle
/* TEST_F(UnitGetUpdate, NoQuanitityChange)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1, 0};
    tagged_limit_order order2{trader1, Ticker::ETH, buy, 1, 2, 0};
    tagged_limit_order order3{trader1, Ticker::ETH, buy, 1, 3, 0};
    before.add_order(order1);
    before.add_order(order2);
    before.add_order(order3);

    tagged_limit_order order4{trader1, Ticker::ETH, buy, 1, 3, 0};
    tagged_limit_order order5{trader1, Ticker::ETH, buy, 1, 2, 0};
    tagged_limit_order order6{trader1, Ticker::ETH, buy, 1, 1, 0};
    after.add_order(order4);
    after.add_order(order5);
    after.add_order(order6);

    auto updates = get_and_reset_updates(Ticker::ETH, before, after);

    ASSERT_EQ(updates.size(), 0);
} */

TEST_F(UnitGetUpdate, BuySellChange)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{trader3, Ticker::ETH, sell, 1.0, 5.0};

    auto o1_it = orderbook_.add_order(order1);
    auto o2_it = orderbook_.add_order(order2);

    orderbook_.change_quantity(o1_it, 4.0);
    orderbook_.change_quantity(o2_it, 4.0);

    auto updates = orderbook_.get_and_reset_updates();

    std::sort(updates.begin(), updates.end(), [](auto a, auto b) {
        return a.price < b.price;
    });

    ASSERT_EQ(updates.size(), 2);
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 5, 1);
    ASSERT_EQ_OB_UPDATE(updates[1], Ticker::ETH, sell, 5, 5);
}

TEST_F(UnitGetUpdate, ManyLevelChanges)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{trader1, Ticker::ETH, buy, 1.0, 2.0};
    tagged_limit_order order3{trader1, Ticker::ETH, buy, 1.0, 3.0};
    tagged_limit_order order4{trader3, Ticker::ETH, buy, 1.0, 4.0};

    orderbook_.add_order(order1);
    orderbook_.add_order(order2);
    orderbook_.add_order(order3);
    orderbook_.add_order(order4);

    orderbook_.get_and_reset_updates();

    tagged_limit_order order5{trader1, Ticker::ETH, buy, 8.0, 2.0};
    tagged_limit_order order6{trader1, Ticker::ETH, buy, 9.0, 3.0};
    tagged_limit_order order7{trader1, Ticker::ETH, buy, 7.0, 4.0};

    orderbook_.add_order(order5);
    orderbook_.add_order(order6);
    orderbook_.add_order(order7);
    auto updates = orderbook_.get_and_reset_updates();

    std::sort(updates.begin(), updates.end(), [](auto a, auto b) {
        return a.price < b.price;
    });

    ASSERT_EQ(updates.size(), 3);
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 9, 2);
    ASSERT_EQ_OB_UPDATE(updates[1], Ticker::ETH, buy, 10, 3);
    ASSERT_EQ_OB_UPDATE(updates[2], Ticker::ETH, buy, 8, 4);
}

TEST_F(UnitGetUpdate, ChangesAddsAndDeletes)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{trader1, Ticker::ETH, buy, 1.0, 2.0};
    tagged_limit_order order3{trader1, Ticker::ETH, buy, 1.0, 3.0};
    tagged_limit_order order4{trader3, Ticker::ETH, buy, 10.0, 4.0};
    tagged_limit_order order5{trader3, Ticker::ETH, buy, 5.0, 5.0};
    tagged_limit_order order6{trader3, Ticker::ETH, buy, 5.0, 5.0};

    orderbook_.add_order(order1);
    orderbook_.add_order(order2);
    orderbook_.add_order(order3);
    orderbook_.add_order(order4);
    orderbook_.add_order(order5);
    auto o6_it = orderbook_.add_order(order6);

    orderbook_.get_and_reset_updates();

    tagged_limit_order order7{trader1, Ticker::ETH, buy, 8.0, 2.0};
    tagged_limit_order order8{trader1, Ticker::ETH, buy, 9.0, 3.0};

    orderbook_.add_order(order7);
    orderbook_.add_order(order8);

    orderbook_.remove_order(o6_it);

    auto updates = orderbook_.get_and_reset_updates();

    std::sort(updates.begin(), updates.end(), [](auto a, auto b) {
        return a.price < b.price;
    });

    ASSERT_EQ(updates.size(), 3);
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 9, 2);
    ASSERT_EQ_OB_UPDATE(updates[1], Ticker::ETH, buy, 10, 3);
    ASSERT_EQ_OB_UPDATE(updates[2], Ticker::ETH, buy, 5, 5);
}
} // namespace nutc::test
