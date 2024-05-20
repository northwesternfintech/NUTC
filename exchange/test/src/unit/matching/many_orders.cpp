#include "config.h"
#include "exchange/traders/trader_container.hpp"
#include "test_utils/helpers/test_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitManyOrders : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;
    std::shared_ptr<nutc::traders::GenericTrader> trader1, trader2, trader3, trader4;

    void
    SetUp() override
    {
        trader1 =
            manager_.add_trader<TestTrader>(std::string("A"), TEST_STARTING_CAPITAL);
        trader2 =
            manager_.add_trader<TestTrader>(std::string("B"), TEST_STARTING_CAPITAL);
        trader3 =
            manager_.add_trader<TestTrader>(std::string("C"), TEST_STARTING_CAPITAL);
        trader4 =
            manager_.add_trader<TestTrader>(std::string("D"), TEST_STARTING_CAPITAL);

        trader1->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        trader2->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        trader3->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        trader4->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    }

    TraderContainer& manager_ = nutc::traders::TraderContainer::get_instance();
    nutc::matching::OrderBook orderbook_;
    Engine engine_{TEST_ORDER_EXPIRATION_TICKS};

    std::vector<nutc::matching::stored_match>
    add_to_engine_(const stored_order& order)
    {
        return engine_.match_order(orderbook_, order);
    }
};

TEST_F(UnitManyOrders, CorrectTimePriority)
{
    stored_order order1{trader1, buy, "ETHUSD", 1, 1, 0};
    stored_order order2{trader2, buy, "ETHUSD", 1, 1, 0};
    stored_order order3{trader3, sell, "ETHUSD", 1, 1, 0};
    stored_order order4{trader4, buy, "ETHUSD", 1, 1, 0};
    stored_order order5{trader3, sell, "ETHUSD", 5, 1, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "A", "C", sell, 1, 1);
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order5);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "B", "C", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "D", "C", sell, 1, 1);
}

TEST_F(UnitManyOrders, OnlyMatchesOne)
{
    stored_order order1{trader1, buy, "ETHUSD", 1, 1, 0};
    stored_order order2{trader2, sell, "ETHUSD", 1, 1, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "A", "B", sell, 1, 1);
}

TEST_F(UnitManyOrders, SimpleManyOrder)
{
    stored_order order1{trader1, buy, "ETHUSD", 1, 1, 0};
    stored_order order2{trader2, buy, "ETHUSD", 1, 1, 0};
    stored_order order3{trader3, buy, "ETHUSD", 1, 1, 0};
    stored_order order4{trader4, sell, "ETHUSD", 3, 1, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 3);

    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "A", "D", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "B", "D", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[2], "ETHUSD", "C", "D", sell, 1, 1);
}

TEST_F(UnitManyOrders, PassiveAndAggressivePartial)
{
    stored_order order1{trader1, sell, "ETHUSD", 1, 1, 0};
    stored_order order2{trader2, sell, "ETHUSD", 10, 1, 0};
    stored_order order3{trader3, buy, "ETHUSD", 2, 3, 0};
    stored_order order4{trader4, buy, "ETHUSD", 10, 4, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "C", "A", buy, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "C", "B", buy, 1, 1);
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "D", "B", buy, 1, 9);
}
