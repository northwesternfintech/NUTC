#include "common/types/decimal.hpp"
#include "config.h"
#include "exchange/matching/engine.hpp"
#include "exchange/orders/orderbook/composite_orderbook.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

using nutc::common::Ticker;
using nutc::common::Side::buy;
using nutc::common::Side::sell;

class UnitMatchIOC : public ::testing::Test {
protected:
    using TestTrader = nutc::test::TestTrader;
    static constexpr nutc::common::decimal_quantity DEFAULT_QUANTITY = 1000.0;

    TestTrader trader1{"ABC", TEST_STARTING_CAPITAL};
    TestTrader trader2{"DEF", TEST_STARTING_CAPITAL};

    void
    SetUp() override
    {
        trader1.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader2.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    }

    nutc::exchange::CompositeOrderBook orderbook_{Ticker::ETH};

    std::vector<nutc::exchange::tagged_match>
    add_to_engine_(const tagged_limit_order& order)
    {
        return nutc::exchange::match_order(order, orderbook_);
    }
};

TEST_F(UnitMatchIOC, BasicMatchIOC)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0, false};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, true};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
}

TEST_F(UnitMatchIOC, DoubleIOCMatchMultipleLevels)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 5.0, 2.0, true};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, false};
    tagged_limit_order order3{trader2, Ticker::ETH, sell, 4.0, 1.0, false};

    auto matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order3);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 2);
}

TEST_F(UnitMatchIOC, NoMatchAfterCycle)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0, false};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, true};

    add_to_engine_(order2);

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());
}

TEST_F(UnitMatchIOC, IOCBuyWithInsufficientQuantityToMatch)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 5.0, 1.0, true};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 10.0, 1.0, false};

    auto matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "DEF", buy, 5.0, 1.0);
}

TEST_F(UnitMatchIOC, IOCSellWithNoMatchingBuyOrders)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 0.5, false};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, true};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());
}

TEST_F(UnitMatchIOC, IOCBuyWithPartialMatch)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 2.0, 1.0, true};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, false};

    auto matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 1);
}

TEST_F(UnitMatchIOC, MultipleIOCOrdersLeadingToNoMatch)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 0.5, true};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.5, true};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());
}

TEST_F(UnitMatchIOC, DoubleIOCOrdersWithNoMatch)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0, true};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, true};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());
}

TEST_F(UnitMatchIOC, IOCBuyOrderPriceImprovement)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1.0, 1.0, false};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1.0, 1.5, true};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ(matches[0].price, 1.0);
}

TEST_F(UnitMatchIOC, IOCSellOrderPriceImprovement)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.5, false};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, true};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ(matches[0].price, 1.5);
}

TEST_F(UnitMatchIOC, MultipleIOCOrdersMatchingAtDifferentPriceLevels)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1.0, 1.0, false};
    tagged_limit_order order2{trader1, Ticker::ETH, sell, 1.0, 2.0, false};
    tagged_limit_order order3{trader2, Ticker::ETH, buy, 2.0, 2.0, true};

    add_to_engine_(order1);
    add_to_engine_(order2);

    auto matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
}

TEST_F(UnitMatchIOC, IOCOrderWithInsufficientFunds)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0, false};
    trader2.get_portfolio().modify_holdings(
        Ticker::ETH, -DEFAULT_QUANTITY
    ); // Remove all ETH holdings from trader2
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, true};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());
}

TEST_F(UnitMatchIOC, IOCOrderWithNoCounterpartOrders)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0, true};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());
}

TEST_F(UnitMatchIOC, IOCOrderWithNoCounterpartOrdersAndInsufficientFunds)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0, true};
    trader1.get_portfolio().modify_holdings(
        Ticker::ETH, -DEFAULT_QUANTITY
    ); // Remove all ETH holdings from trader1

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());
}

TEST_F(UnitMatchIOC, MultipleIOCOrdersWithNoCounterpartOrders)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0, true};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, true};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());
}

TEST_F(UnitMatchIOC, MultipleIOCOrdersWithNoCounterpartOrdersAndInsufficientFunds)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0, true};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, true};
    trader1.get_portfolio().modify_holdings(
        Ticker::ETH, -DEFAULT_QUANTITY
    ); // Remove all ETH holdings from trader1
    trader2.get_portfolio().modify_holdings(
        Ticker::ETH, -DEFAULT_QUANTITY
    ); // Remove all ETH holdings from trader2

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());
}

TEST_F(UnitMatchIOC, VerifyOrderBookAfterMatch)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0, false};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, true};

    add_to_engine_(order1);
    add_to_engine_(order2);

    auto buy_orders = orderbook_.get_top_order(buy);
    ASSERT_FALSE(buy_orders.has_value());

    auto sell_orders = orderbook_.get_top_order(sell);
    ASSERT_FALSE(sell_orders.has_value());
}

TEST_F(UnitMatchIOC, VerifyOrderBookAfterPartialMatch)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 2.0, 1.0, false};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, true};

    add_to_engine_(order1);
    add_to_engine_(order2);

    auto buy_orders = orderbook_.get_top_order(buy);
    ASSERT_TRUE(buy_orders.has_value());
    ASSERT_EQ((*buy_orders)->quantity, 1.0);

    auto sell_orders = orderbook_.get_top_order(sell);
    ASSERT_FALSE(sell_orders.has_value());
}

TEST_F(UnitMatchIOC, VerifyOrderBookAfterNoMatch)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0, false};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 2.0, true};

    add_to_engine_(order1);
    add_to_engine_(order2);

    auto buy_orders = orderbook_.get_top_order(buy);
    ASSERT_TRUE(buy_orders.has_value());
    ASSERT_EQ((*buy_orders)->quantity, 1.0);

    auto sell_orders = orderbook_.get_top_order(sell);
    ASSERT_FALSE(sell_orders.has_value());
}

TEST_F(UnitMatchIOC, VerifyOrderBookAfterMultipleLevelsMatch)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 5.0, 2.0, true};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0, false};
    tagged_limit_order order3{trader2, Ticker::ETH, sell, 4.0, 1.0, false};

    add_to_engine_(order2);
    add_to_engine_(order3);
    add_to_engine_(order1);

    auto buy_orders = orderbook_.get_top_order(buy);
    ASSERT_FALSE(buy_orders.has_value());

    auto sell_orders = orderbook_.get_top_order(sell);
    ASSERT_FALSE(sell_orders.has_value());
}
