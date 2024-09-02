#include "exchange/orders/orderbook/composite_orderbook.hpp"

#include "common/types/decimal.hpp"
#include "config.h"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

// TODO: expiration tests

using nutc::common::Ticker;
using nutc::common::Side::buy;
using nutc::common::Side::sell;

class UnitCompositeOrderBookTest : public ::testing::Test {
protected:
    using TestTrader = nutc::test::TestTrader;
    static constexpr nutc::common::decimal_quantity DEFAULT_QUANTITY = 1000.0;

    TestTrader trader_1{"ABC", TEST_STARTING_CAPITAL};
    TestTrader trader_2{"DEF", TEST_STARTING_CAPITAL};

    void
    SetUp() override
    {
        trader_1.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader_2.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    }

    nutc::exchange::CompositeOrderBook container_{Ticker::ETH};
};

TEST_F(UnitCompositeOrderBookTest, TestStorageRounding)
{
    tagged_limit_order order1{trader_1, Ticker::ETH, buy, 1.0, 1.000001};
    EXPECT_EQ(order1.price, 1.0);

    tagged_limit_order order2{trader_2, Ticker::ETH, buy, 0.00001, .9999, 1};
    EXPECT_EQ(order2.price, 1.0);

    tagged_limit_order order3{trader_2, Ticker::ETH, buy, 1.0, .994};
    EXPECT_EQ(order3.price, 0.99);
}

TEST_F(UnitCompositeOrderBookTest, SimpleAddRemove)
{
    tagged_limit_order order1{trader_1, Ticker::ETH, buy, 1.0, 1.0};

    auto order1_it = container_.add_order(order1);
    auto updates = container_.get_and_reset_updates();

    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 1, 1);

    container_.remove_order(order1_it);
    updates = container_.get_and_reset_updates();
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 0, 1);
}

TEST_F(UnitCompositeOrderBookTest, RemoveOrderById)
{
    tagged_limit_order order1{trader_1, Ticker::ETH, buy, 1.0, 1.0};

    container_.add_order(order1);

    auto updates = container_.get_and_reset_updates();
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 1, 1);

    container_.remove_order(order1.order_id);
    updates = container_.get_and_reset_updates();
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 0, 1);

    EXPECT_FALSE(container_.get_top_order(buy).has_value());
}

TEST_F(UnitCompositeOrderBookTest, RemoveOrderByIterator)
{
    tagged_limit_order order{trader_1, Ticker::ETH, buy, 1.0, 1.0};

    auto order_it = container_.add_order(order);

    auto updates = container_.get_and_reset_updates();
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 1, 1);

    container_.remove_order(order_it);
    updates = container_.get_and_reset_updates();
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 0, 1);

    EXPECT_FALSE(container_.get_top_order(buy).has_value());
}

TEST_F(UnitCompositeOrderBookTest, RemoveNonExistingOrder)
{
    tagged_limit_order order{trader_1, Ticker::ETH, buy, 1.0, 1.0};

    container_.add_order(order);

    auto updates = container_.get_and_reset_updates();
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 1, 1);

    container_.remove_order(order.order_id + 1);
    updates = container_.get_and_reset_updates();
    EXPECT_TRUE(updates.empty());

    EXPECT_TRUE(container_.get_top_order(buy).has_value());
}

TEST_F(UnitCompositeOrderBookTest, GetMidpriceTest)
{
    tagged_limit_order order1{trader_1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{trader_2, Ticker::ETH, sell, 1.0, 1.0};

    container_.add_order(order1);
    container_.add_order(order2);

    EXPECT_EQ(container_.get_midprice(), 1.0);

    container_.remove_order(order1.order_id);
    EXPECT_EQ(container_.get_midprice(), 0.0);
}

TEST_F(UnitCompositeOrderBookTest, ChangeQuantity)
{
    tagged_limit_order order{trader_1, Ticker::ETH, buy, 1.0, 1.0};

    auto order_it = container_.add_order(order);

    auto updates = container_.get_and_reset_updates();
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 1, 1);

    container_.change_quantity(order_it, 2.0);
    updates = container_.get_and_reset_updates();
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 3, 1);
}

TEST_F(UnitCompositeOrderBookTest, ChangeQuantityRemovesOrderWhenZero)
{
    tagged_limit_order order{trader_1, Ticker::ETH, buy, 1.0, 1.0};

    auto order_it = container_.add_order(order);

    auto updates = container_.get_and_reset_updates();
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 1, 1);

    container_.change_quantity(order_it, -order.quantity);
    updates = container_.get_and_reset_updates();
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 0, 1);

    EXPECT_FALSE(container_.get_top_order(buy).has_value());
}
