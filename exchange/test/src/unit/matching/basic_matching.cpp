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
using namespace nutc::test;

class UnitBasicMatching : public ::testing::Test {
protected:
    using TestTrader = nutc::test::TestTrader;
    static constexpr nutc::common::decimal_quantity DEFAULT_QUANTITY = 1000.0;

    TestTrader trader1{"ABC", TEST_STARTING_CAPITAL};
    TestTrader trader2{"DEF", TEST_STARTING_CAPITAL};
    TestTrader trader3{"GHI", TEST_STARTING_CAPITAL};

    void
    SetUp() override
    {
        trader1.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader2.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader3.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    }

    void
    TearDown() override
    {
        SetUp();
    }

    nutc::exchange::CompositeOrderBook orderbook_{Ticker::ETH};

    std::vector<nutc::common::match>
    add_to_engine_(const tagged_limit_order& order)
    {
        return nutc::exchange::match_order(order, orderbook_);
    }
};

TEST_F(UnitBasicMatching, SimpleMatch)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "DEF", sell, 1, 1);
}

TEST_F(UnitBasicMatching, CorrectBuyPricingOrder)
{
    tagged_limit_order buy1{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order buy2{trader1, Ticker::ETH, buy, 1.0, 2.0};
    tagged_limit_order buy3{trader1, Ticker::ETH, buy, 1.0, 3.0};
    tagged_limit_order buy4{trader1, Ticker::ETH, buy, 1.0, 4.0};
    tagged_limit_order sell1{trader2, Ticker::ETH, sell, 1.0, 1.0};

    // Place cheapest buy orders first, then most expensive
    auto matches = add_to_engine_(buy1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(buy3);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(buy2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(buy4);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(sell1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "DEF", sell, 1, 4);
    matches = add_to_engine_(sell1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "DEF", sell, 1, 3);
}

TEST_F(UnitBasicMatching, NoMatchThenMatchBuy)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1.0, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0};
    tagged_limit_order order3{trader2, Ticker::ETH, buy, 1.0, 2.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "DEF", "ABC", buy, 1, 1);
}

TEST_F(UnitBasicMatching, NoMatchThenMatchSell)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order3{trader3, Ticker::ETH, sell, 2.0, 0.0};
    tagged_limit_order order4{trader1, Ticker::ETH, buy, 1.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "GHI", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], Ticker::ETH, "DEF", "GHI", sell, 1, 1);
}

TEST_F(UnitBasicMatching, PassivePriceMatch)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 2.0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "DEF", sell, 1, 2);
}

TEST_F(UnitBasicMatching, PartialFill)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 2.0, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "ABC", "DEF", sell, 1, 1);
}

TEST_F(UnitBasicMatching, MultipleFill)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order3{trader2, Ticker::ETH, sell, 2.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "ABC", "DEF", sell, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), Ticker::ETH, "ABC", "DEF", sell, 1, 1);
}

TEST_F(UnitBasicMatching, MultiplePartialFill)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order3{trader2, Ticker::ETH, sell, 3.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "ABC", "DEF", sell, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), Ticker::ETH, "ABC", "DEF", sell, 1, 1);
}

TEST_F(UnitBasicMatching, SimpleMatchReversed)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1.0, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1.0, 1.0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "DEF", "ABC", buy, 1, 1);
}

TEST_F(UnitBasicMatching, PassivePriceMatchReversed)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1.0, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1.0, 2.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ(matches.at(0).position.price, 1.0);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "DEF", "ABC", buy, 1, 1);
}

TEST_F(UnitBasicMatching, PartialFillReversed)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 2.0, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "DEF", "ABC", buy, 1, 1);
}

TEST_F(UnitBasicMatching, MultipleFillReversed)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1.0, 1.0};
    tagged_limit_order order2{trader1, Ticker::ETH, sell, 1.0, 1.0};
    tagged_limit_order order3{trader2, Ticker::ETH, buy, 2.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "DEF", "ABC", buy, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), Ticker::ETH, "DEF", "ABC", buy, 1, 1);
}

TEST_F(UnitBasicMatching, MultiplePartialFillReversed)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1.0, 1.0};
    tagged_limit_order order2{trader1, Ticker::ETH, sell, 1.0, 1.0};
    tagged_limit_order order3{trader2, Ticker::ETH, buy, 3.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);

    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "DEF", "ABC", buy, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), Ticker::ETH, "DEF", "ABC", buy, 1, 1);
}

TEST_F(UnitBasicMatching, MultipleOrders)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order3{trader2, Ticker::ETH, sell, 1.0, 1.0};
    tagged_limit_order order4{trader2, Ticker::ETH, sell, 1.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "ABC", "DEF", sell, 1, 1);

    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "ABC", "DEF", sell, 1, 1);
}

TEST_F(UnitBasicMatching, VerifyOrderbook)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order2{trader1, Ticker::ETH, buy, 1.0, 1.0};
    tagged_limit_order order3{trader2, Ticker::ETH, sell, 1.0, 1.0};
    tagged_limit_order order4{trader2, Ticker::ETH, sell, 1.0, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "ABC", "DEF", sell, 1, 1);

    auto buy_order = orderbook_.get_top_order(buy);
    ASSERT_TRUE(buy_order.has_value());
    ASSERT_EQ((*buy_order)->quantity, 1.0);

    auto sell_order = orderbook_.get_top_order(sell);
    ASSERT_FALSE(sell_order.has_value());
}

TEST_F(UnitBasicMatching, VerifyOrderbookAfterPartialMatch)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 2.0, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1.0, 1.0};

    add_to_engine_(order1);
    add_to_engine_(order2);

    auto buy_orders = orderbook_.get_top_order(buy);
    ASSERT_TRUE(buy_orders.has_value());
    ASSERT_EQ((*buy_orders)->quantity, 1.0);

    auto sell_orders = orderbook_.get_top_order(sell);
    ASSERT_FALSE(sell_orders.has_value());
}
