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

class UnitMatchMarket : public ::testing::Test {
protected:
    using TestTrader = nutc::test::TestTrader;
    static constexpr nutc::common::decimal_quantity DEFAULT_QUANTITY = 1000.0;

    TestTrader trader1{"ABC", TEST_STARTING_CAPITAL};
    TestTrader trader2{"DEF", TEST_STARTING_CAPITAL};
    TestTrader trader3{"GHI", TEST_STARTING_CAPITAL};
    TestTrader trader4{"JKL", TEST_STARTING_CAPITAL};

    void
    SetUp() override
    {
        trader1.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader2.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader3.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader4.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    }

    nutc::exchange::CompositeOrderBook orderbook_{Ticker::ETH};

    std::vector<nutc::common::match>
    add_to_engine_(const auto& order)
    {
        return nutc::exchange::match_order(order, orderbook_);
    }
};

TEST_F(UnitMatchMarket, BasicMatchMarket)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 1.0, false};
    tagged_market_order order2{trader2, Ticker::ETH, sell, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
}

TEST_F(UnitMatchMarket, DoubleMarketMatchMultipleLevels)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 5.0, 2.0, true};
    tagged_market_order order2{trader2, Ticker::ETH, sell, 1.0};
    tagged_market_order order3{trader2, Ticker::ETH, sell, 4.0};

    auto matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order3);
    ASSERT_TRUE(matches.empty());
}

TEST_F(UnitMatchMarket, MarketMatchLimit)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 5.0, 2.0, false};
    tagged_market_order order2{trader2, Ticker::ETH, sell, 5.0};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
}

TEST_F(UnitMatchMarket, MarketMatchMultipleLevels)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 2.0};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1.0, 2.0};
    tagged_limit_order order3{trader3, Ticker::ETH, buy, 1.0, 2.0};
    tagged_market_order order4{trader4, Ticker::ETH, sell, 5.0};

    add_to_engine_(order1);
    add_to_engine_(order2);
    add_to_engine_(order3);

    auto matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 3);
}

TEST_F(UnitMatchMarket, MarketMatchSomeLevels)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 2.0};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1.0, 2.0};
    tagged_limit_order order3{trader3, Ticker::ETH, buy, 1.0, 2.0};
    tagged_market_order order4{trader4, Ticker::ETH, sell, 2.0};

    add_to_engine_(order1);
    add_to_engine_(order2);
    add_to_engine_(order3);

    auto matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 2);
}

TEST_F(UnitMatchMarket, VerifyIOCDoesNotMatchWithMarket)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 2.0, true};
    tagged_market_order order2{trader2, Ticker::ETH, sell, 2.0};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());
}

TEST_F(UnitMatchMarket, VerifyMarketDoesNotMatchWithIOC)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1.0, 2.0, true};
    tagged_market_order order2{trader2, Ticker::ETH, sell, 2.0};

    auto matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());
}

TEST_F(UnitMatchMarket, VerifyMarketDoesNotMatchWithMarket)
{
    tagged_market_order order1{trader1, Ticker::ETH, buy, 2.0};
    tagged_market_order order2{trader2, Ticker::ETH, sell, 2.0};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());
}
