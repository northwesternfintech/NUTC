#include "common/types/decimal.hpp"
#include "config.h"
#include "exchange/orders/orderbook/composite_orderbook.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

// TODO: expiration tests

using nutc::common::Ticker;
using nutc::common::Side::buy;
using nutc::common::Side::sell;

class UnitOrderBookTest : public ::testing::Test {
protected:
    using TestTrader = nutc::test::TestTrader;
    static constexpr nutc::common::decimal_quantity DEFAULT_QUANTITY = 1000.0;

    TraderContainer traders;

    nutc::exchange::GenericTrader& trader_1 =
        *traders.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
    nutc::exchange::GenericTrader& trader_2 =
        *traders.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);

    void
    SetUp() override
    {
        trader_1.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader_2.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    }

    nutc::exchange::CompositeOrderBook container_{Ticker::ETH};
};

TEST_F(UnitOrderBookTest, TestStorageRounding)
{
    tagged_limit_order order1{trader_1, Ticker::ETH, buy, 1, 1.000001};
    ASSERT_EQ(order1.price, 1.0);

    tagged_limit_order order2{trader_2, Ticker::ETH, buy, 0.00001, .9999, 1};
    ASSERT_EQ(order2.price, 1.0);

    tagged_limit_order order3{trader_2, Ticker::ETH, buy, 1, .994};
    ASSERT_EQ(order3.price, 0.99);
}

TEST_F(UnitOrderBookTest, SimpleAddRemove)
{
    tagged_limit_order order1{trader_1, Ticker::ETH, buy, 1, 1.0};
    // tagged_limit_order order2{trader_2, Ticker::ETH, sell, 1, 1.0};

    container_.add_order(order1);
    // ASSERT_EQ(container_.get_level(buy, 1.0), 1);
    // ASSERT_FALSE(container_.can_match_orders());
    //
    // container_.add_order(order2);
    // ASSERT_EQ(container_.get_level(sell, 1.0), 1);
    // ASSERT_TRUE(container_.can_match_orders());

    // auto order = container_.remove_order(order1.order_index);
    // ASSERT_EQ(order.order_index, order1.order_index);
    // ASSERT_EQ(order.price, order1.price);
    // ASSERT_EQ(container_.get_level(buy, 1.0), 0);
    // ASSERT_FALSE(container_.can_match_orders());
}

TEST_F(UnitOrderBookTest, ModifyQuantity)
{
    tagged_limit_order so1{trader_1, Ticker::ETH, buy, 1, 1.0};
    tagged_limit_order so2{trader_1, Ticker::ETH, sell, 1, 1.0};

    container_.add_order(so1);
    container_.add_order(so2);
    // ASSERT_EQ(container_.get_level(sell, 1.0), 1);
    // ASSERT_TRUE(container_.can_match_orders());

    // container_.modify_order_quantity(so2.order_index, 1);
    // ASSERT_EQ(container_.get_level(sell, 1.0), 2);
    //
    // container_.modify_order_quantity(so2.order_index, -2);
    // ASSERT_EQ(container_.get_level(sell, 1.0), 0);
    // ASSERT_FALSE(container_.can_match_orders());
}
