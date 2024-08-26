#include "config.h"
#include "exchange/traders/trader_container.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

using nutc::shared::Ticker;
using nutc::shared::Side::buy;
using nutc::shared::Side::sell;

class UnitManyOrders : public ::testing::Test {
protected:
    using TestTrader = nutc::test::TestTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;
    TraderContainer traders;

    nutc::exchange::GenericTrader& trader1 =
        *traders.add_trader<TestTrader>(std::string("A"), TEST_STARTING_CAPITAL);
    nutc::exchange::GenericTrader& trader2 =
        *traders.add_trader<TestTrader>(std::string("B"), TEST_STARTING_CAPITAL);
    nutc::exchange::GenericTrader& trader3 =
        *traders.add_trader<TestTrader>(std::string("C"), TEST_STARTING_CAPITAL);
    nutc::exchange::GenericTrader& trader4 =
        *traders.add_trader<TestTrader>(std::string("D"), TEST_STARTING_CAPITAL);

    void
    SetUp() override
    {
        trader1.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader2.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader3.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader4.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    }

    nutc::exchange::LimitOrderBook orderbook_;
    Engine engine_;

    std::vector<nutc::shared::match>
    add_to_engine_(const tagged_limit_order& order)
    {
        return engine_.match_order(order, orderbook_);
    }
};

TEST_F(UnitManyOrders, CorrectTimePriority)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1, 1.0};
    tagged_limit_order order3{trader3, Ticker::ETH, sell, 1, 1.0};
    tagged_limit_order order4{trader4, Ticker::ETH, buy, 1, 1.0};
    tagged_limit_order order5{trader3, Ticker::ETH, sell, 5, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "A", "C", sell, 1, 1);
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order5);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "B", "C", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], Ticker::ETH, "D", "C", sell, 1, 1);
}

TEST_F(UnitManyOrders, OnlyMatchesOne)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "A", "B", sell, 1, 1);
}

TEST_F(UnitManyOrders, SimpleManyOrder)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1, 1.0};
    tagged_limit_order order3{trader3, Ticker::ETH, buy, 1, 1.0};
    tagged_limit_order order4{trader4, Ticker::ETH, sell, 3, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 3);

    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "A", "D", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], Ticker::ETH, "B", "D", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[2], Ticker::ETH, "C", "D", sell, 1, 1);
}

TEST_F(UnitManyOrders, PassiveAndAggressivePartial)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1, 1.0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 10, 1.0};
    tagged_limit_order order3{trader3, Ticker::ETH, buy, 2, 3.0};
    tagged_limit_order order4{trader4, Ticker::ETH, buy, 10, 4.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "C", "A", buy, 1, 1);
    ASSERT_EQ_MATCH(matches[1], Ticker::ETH, "C", "B", buy, 1, 1);
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "D", "B", buy, 9, 1);
}
