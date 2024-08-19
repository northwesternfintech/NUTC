#include "config.h"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/traders/trader_container.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

// TODO: expiration tests

using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitOrderBookTest : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;

    TraderContainer traders;

    nutc::traders::GenericTrader& trader_1 =
        *traders.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader_2 =
        *traders.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);

    void
    SetUp() override
    {
        trader_1.modify_holdings("ETH", DEFAULT_QUANTITY);
        trader_2.modify_holdings("ETH", DEFAULT_QUANTITY);
    }

    nutc::matching::LimitOrderBook container_;
};

TEST_F(UnitOrderBookTest, TestStorageRounding)
{
    stored_order order1{trader_1, "ETH", buy, 1, 1.000001};
    ASSERT_EQ(order1.position.price, 1.0);

    stored_order order2{trader_2, "ETH", buy, 0.00001, .9999, 1};
    ASSERT_EQ(order2.position.price, 1.0);

    stored_order order3{trader_2, "ETH", buy, 1, .994};
    ASSERT_EQ(order3.position.price, 0.99);
}

TEST_F(UnitOrderBookTest, SimpleAddRemove)
{
    stored_order order1{trader_1, "ETH", buy, 1, 1, 1};
    // stored_order order2{trader_2, "ETH", sell, 1, 1, 1};

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
    stored_order so1{trader_1, "ETH", buy, 1, 1, 1};
    stored_order so2{trader_1, "ETH", sell, 1, 1, 1};

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
