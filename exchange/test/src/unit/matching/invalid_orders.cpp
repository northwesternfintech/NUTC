#include "config.h"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "shared/util.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitInvalidOrders : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;
    TraderContainer manager_;
    nutc::traders::GenericTrader& trader1 =
        *manager_.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader2 =
        *manager_.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);

    void
    SetUp() override
    {
        trader1.modify_holdings("ETH", DEFAULT_QUANTITY);
        trader2.modify_holdings("ETH", DEFAULT_QUANTITY);
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

TEST_F(UnitInvalidOrders, RemoveThenAddFunds)
{
    trader1.modify_capital(-TEST_STARTING_CAPITAL);

    stored_order order2{trader2, "ETH", sell, 1, 1, 0};
    stored_order order1{trader1, "ETH", buy, 1, 1, 0};

    // Thrown out
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    // Kept, but not matched
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    trader1.modify_capital(TEST_STARTING_CAPITAL);

    // Kept, but not matched
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    // Kept and matched
    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETH", "ABC", "DEF", buy, 1, 1);
}

TEST_F(UnitInvalidOrders, MatchingInvalidFunds)
{
    trader1.modify_capital(-TEST_STARTING_CAPITAL);

    stored_order order1{trader1, "ETH", buy, 1, 1, 0};
    stored_order order2{trader2, "ETH", sell, 1, 1, 0};

    // Thrown out
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    // Kept, but not matched
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
}

TEST_F(UnitInvalidOrders, SimpleManyInvalidOrder)
{
    nutc::traders::GenericTrader& t1 =
        *(manager_.add_trader<TestTrader>(std::string("A"), TEST_STARTING_CAPITAL));
    nutc::traders::GenericTrader& t2 =
        *(manager_.add_trader<TestTrader>(std::string("B"), 0));
    nutc::traders::GenericTrader& t3 =
        *(manager_.add_trader<TestTrader>(std::string("C"), TEST_STARTING_CAPITAL));
    nutc::traders::GenericTrader& t4 =
        *(manager_.add_trader<TestTrader>(std::string("D"), TEST_STARTING_CAPITAL));

    t1.modify_holdings("ETH", DEFAULT_QUANTITY);
    t2.modify_holdings("ETH", DEFAULT_QUANTITY);
    t3.modify_holdings("ETH", DEFAULT_QUANTITY);
    t4.modify_holdings("ETH", DEFAULT_QUANTITY);

    stored_order order1{t1, "ETH", buy, 1, 1, 0};
    stored_order order2{t2, "ETH", buy, 1, 1, 0};
    stored_order order3{t3, "ETH", buy, 1, 1, 0};
    stored_order order4{t4, "ETH", sell, 3, 1, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 0);

    // Should match two orders and throw out the invalid order (2)
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 2);

    ASSERT_EQ_MATCH(matches[0], "ETH", "A", "D", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETH", "C", "D", sell, 1, 1);
}
