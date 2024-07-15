#include "config.h"
#include "exchange/traders/trader_container.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitManyOrders : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;
    TraderContainer& manager_ = nutc::traders::TraderContainer::get_instance();

    nutc::traders::GenericTrader& trader1 =
        *manager_.add_trader<TestTrader>(std::string("A"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader2 =
        *manager_.add_trader<TestTrader>(std::string("B"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader3 =
        *manager_.add_trader<TestTrader>(std::string("C"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader4 =
        *manager_.add_trader<TestTrader>(std::string("D"), TEST_STARTING_CAPITAL);

    void
    SetUp() override
    {
        trader1.modify_holdings("ETH", DEFAULT_QUANTITY);
        trader2.modify_holdings("ETH", DEFAULT_QUANTITY);
        trader3.modify_holdings("ETH", DEFAULT_QUANTITY);
        trader4.modify_holdings("ETH", DEFAULT_QUANTITY);
    }

    nutc::matching::LimitOrderBook orderbook_;
    Engine engine_;

    std::vector<nutc::matching::stored_match>
    add_to_engine_(const stored_order& order)
    {
		orderbook_.add_order(order);
        return engine_.match_orders(orderbook_);
    }
};

TEST_F(UnitManyOrders, CorrectTimePriority)
{
    stored_order order1{trader1, "ETH", buy, 1, 1, 0};
    stored_order order2{trader2, "ETH", buy, 1, 1, 0};
    stored_order order3{trader3, "ETH", sell, 1, 1, 0};
    stored_order order4{trader4, "ETH", buy, 1, 1, 0};
    stored_order order5{trader3, "ETH", sell, 1, 5, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETH", "A", "C", sell, 1, 1);
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order5);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], "ETH", "B", "C", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETH", "D", "C", sell, 1, 1);
}

TEST_F(UnitManyOrders, OnlyMatchesOne)
{
    stored_order order1{trader1, "ETH", buy, 1, 1, 0};
    stored_order order2{trader2, "ETH", sell, 1, 1, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETH", "A", "B", sell, 1, 1);
}

TEST_F(UnitManyOrders, SimpleManyOrder)
{
    stored_order order1{trader1, "ETH", buy, 1, 1, 0};
    stored_order order2{trader2, "ETH", buy, 1, 1, 0};
    stored_order order3{trader3, "ETH", buy, 1, 1, 0};
    stored_order order4{trader4, "ETH", sell, 1, 3, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 3);

    ASSERT_EQ_MATCH(matches[0], "ETH", "A", "D", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETH", "B", "D", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[2], "ETH", "C", "D", sell, 1, 1);
}

TEST_F(UnitManyOrders, PassiveAndAggressivePartial)
{
    stored_order order1{trader1, "ETH", sell, 1, 1, 0};
    stored_order order2{trader2, "ETH", sell, 1, 10, 0};
    stored_order order3{trader3, "ETH", buy, 3, 2, 0};
    stored_order order4{trader4, "ETH", buy, 4, 10, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], "ETH", "C", "A", buy, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETH", "C", "B", buy, 1, 1);
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETH", "D", "B", buy, 1, 9);
}
