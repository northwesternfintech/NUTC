#include "common/types/decimal.hpp"
#include "common/util.hpp"
#include "config.h"
#include "exchange/matching/engine.hpp"
#include "exchange/orders/orderbook/composite_orderbook.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

using nutc::common::Ticker;
using nutc::common::Side::buy;
using nutc::common::Side::sell;

class UnitInvalidOrders : public ::testing::Test {
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

    std::vector<nutc::common::match>
    add_to_engine_(const tagged_limit_order& order)
    {
        return nutc::exchange::match_order(order, orderbook_);
    }
};

TEST_F(UnitInvalidOrders, RemoveThenAddFunds)
{
    trader1.get_portfolio().modify_capital(-TEST_STARTING_CAPITAL);

    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order11{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0};
    tagged_limit_order order22{trader2, Ticker::ETH, sell, 1.0, 1.0};

    // Thrown out
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    // Kept, but not matched
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    trader1.get_portfolio().modify_capital(TEST_STARTING_CAPITAL);

    // Kept, but not matched
    matches = add_to_engine_(order22);
    ASSERT_EQ(matches.size(), 0);

    // Kept and matched
    matches = add_to_engine_(order11);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "DEF", buy, 1, 1);
}

TEST_F(UnitInvalidOrders, MatchingInvalidFunds)
{
    trader1.get_portfolio().modify_capital(-TEST_STARTING_CAPITAL);

    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0};

    // Thrown out
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    // Kept, but not matched
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
}

TEST_F(UnitInvalidOrders, SimpleManyInvalidOrder)
{
    TestTrader t1{"A", TEST_STARTING_CAPITAL};
    TestTrader t2{"B", 0};
    TestTrader t3{"C", TEST_STARTING_CAPITAL};
    TestTrader t4{"D", TEST_STARTING_CAPITAL};

    t1.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    t2.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    t3.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    t4.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);

    tagged_limit_order order1{t1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{t2, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order3{t3, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order4{t4, Ticker::ETH, sell, 3.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 0);

    // Should match two orders and throw out the invalid order (2)
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 2);

    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "A", "D", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], Ticker::ETH, "C", "D", sell, 1, 1);
}

TEST_F(UnitInvalidOrders, InvalidSellerHoldings)
{
    TestTrader t1{"A", TEST_STARTING_CAPITAL};
    TestTrader t2{"B", TEST_STARTING_CAPITAL};

    t1.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    t2.get_portfolio().modify_holdings(Ticker::ETH, -DEFAULT_QUANTITY);

    tagged_limit_order order1{t1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{t2, Ticker::ETH, sell, 1.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    t2.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY * 2.0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
}

TEST_F(UnitInvalidOrders, InvalidBuyerHoldingsDoesntStopMatch)
{
    TestTrader t1{"A", TEST_STARTING_CAPITAL};
    TestTrader t2{"B", TEST_STARTING_CAPITAL};

    t1.get_portfolio().modify_holdings(Ticker::ETH, -DEFAULT_QUANTITY);
    t2.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);

    tagged_limit_order order1{t1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{t2, Ticker::ETH, sell, 1.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
}
