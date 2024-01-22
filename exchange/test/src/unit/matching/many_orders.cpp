#include "shared/messages.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitManyOrders : public ::testing::Test {
protected:
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        using nutc::testing_utils::add_client_simple;

        add_client_simple(manager_, "A");
        add_client_simple(manager_, "B");
        add_client_simple(manager_, "C");
        add_client_simple(manager_, "D");

        manager_.modify_holdings("A", "ETHUSD", DEFAULT_QUANTITY);
        manager_.modify_holdings("B", "ETHUSD", DEFAULT_QUANTITY);
        manager_.modify_holdings("C", "ETHUSD", DEFAULT_QUANTITY);
        manager_.modify_holdings("D", "ETHUSD", DEFAULT_QUANTITY);
    }

    ClientManager manager_; // NOLINT (*)
    Engine engine_;         // NOLINT (*)
};

TEST_F(UnitManyOrders, CorrectTimePriority)
{
    MarketOrder order1{"A", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"B", BUY, "ETHUSD", 1, 1};
    MarketOrder order3{"C", SELL, "ETHUSD", 1, 1};
    MarketOrder order4{"D", BUY, "ETHUSD", 1, 1};

    auto [matches1, updates1] = engine_.match_order(order1, manager_);
    auto [matches2, updates2] = engine_.match_order(order2, manager_);
    engine_.add_order_without_matching(order3);
    auto [matches3, updates3] = engine_.match_order(order4, manager_);
    EXPECT_EQ(matches1.size(), 0);
    EXPECT_EQ(updates1.size(), 1);
    EXPECT_EQ_OB_UPDATE(updates1[0], "ETHUSD", BUY, 1, 1);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(updates2.size(), 1);
    EXPECT_EQ_OB_UPDATE(updates2[0], "ETHUSD", BUY, 1, 1);

    EXPECT_EQ(matches3.size(), 1);
    EXPECT_EQ(updates3.size(), 3);
    EXPECT_EQ_OB_UPDATE(updates3[0], "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates3[1], "ETHUSD", SELL, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates3[2], "ETHUSD", BUY, 1, 1);
    EXPECT_EQ_MATCH(matches3[0], "ETHUSD", "A", "C", SELL, 1, 1);
}

TEST_F(UnitManyOrders, OnlyMatchesOne)
{
    MarketOrder order1{"A", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"B", SELL, "ETHUSD", 1, 1};

    auto [matches1, updates1] = engine_.match_order(order1, manager_);
    auto [matches2, updates2] = engine_.match_order(order1, manager_);
    EXPECT_EQ(matches1.size(), 0);
    EXPECT_EQ(updates1.size(), 1);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(updates2.size(), 1);

    auto [matches3, updates3] = engine_.match_order(order2, manager_);
    EXPECT_EQ(matches3.size(), 1);
    EXPECT_EQ(updates3.size(), 1);
    EXPECT_EQ_MATCH(matches3[0], "ETHUSD", "A", "B", SELL, 1, 1);
    EXPECT_EQ_OB_UPDATE(updates3[0], "ETHUSD", BUY, 1, 0);
}

TEST_F(UnitManyOrders, SimpleManyOrder)
{
    MarketOrder order1{"A", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"B", BUY, "ETHUSD", 1, 1};
    MarketOrder order3{"C", BUY, "ETHUSD", 1, 1};
    MarketOrder order4{"D", SELL, "ETHUSD", 3, 1};

    auto [matches1, updates1] = engine_.match_order(order1, manager_);
    auto [matches2, updates2] = engine_.match_order(order2, manager_);
    auto [matches3, updates3] = engine_.match_order(order3, manager_);

    EXPECT_EQ(matches1.size(), 0);
    EXPECT_EQ(updates1.size(), 1);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(updates2.size(), 1);
    EXPECT_EQ(matches3.size(), 0);
    EXPECT_EQ(updates3.size(), 1);

    auto [matches4, updates4] = engine_.match_order(order4, manager_);
    EXPECT_EQ(matches4.size(), 3);
    EXPECT_EQ(updates4.size(), 3);

    EXPECT_EQ_MATCH(matches4[0], "ETHUSD", "A", "D", SELL, 1, 1);
    EXPECT_EQ_MATCH(matches4[1], "ETHUSD", "B", "D", SELL, 1, 1);
    EXPECT_EQ_MATCH(matches4[2], "ETHUSD", "C", "D", SELL, 1, 1);

    EXPECT_EQ_OB_UPDATE(updates4[0], "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates4[1], "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates4[2], "ETHUSD", BUY, 1, 0);
}

TEST_F(UnitManyOrders, PassiveAndAggressivePartial)
{
    MarketOrder order1{"A", SELL, "ETHUSD", 1, 1};
    MarketOrder order2{"B", SELL, "ETHUSD", 10, 1}; // NOLINT (*)
    MarketOrder order3{"C", BUY, "ETHUSD", 2, 3};
    MarketOrder order4{"D", BUY, "ETHUSD", 10, 4}; // NOLINT (*)

    auto [matches1, updates1] = engine_.match_order(order1, manager_);
    auto [matches2, updates2] = engine_.match_order(order2, manager_);
    auto [matches3, updates3] = engine_.match_order(order3, manager_);
    auto [matches4, updates4] = engine_.match_order(order4, manager_);

    EXPECT_EQ(matches1.size(), 0);
    EXPECT_EQ(updates1.size(), 1);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(updates2.size(), 1);
    EXPECT_EQ(matches3.size(), 2);
    EXPECT_EQ(updates3.size(), 3);
    EXPECT_EQ(matches4.size(), 1);
    EXPECT_EQ(updates4.size(), 2);

    EXPECT_EQ_MATCH(matches3[0], "ETHUSD", "C", "A", BUY, 1, 1);
    EXPECT_EQ_MATCH(matches3[1], "ETHUSD", "C", "B", BUY, 1, 1);
    EXPECT_EQ_OB_UPDATE(updates3[0], "ETHUSD", SELL, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates3[1], "ETHUSD", SELL, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates3[2], "ETHUSD", SELL, 1, 9);

    EXPECT_EQ_MATCH(matches4[0], "ETHUSD", "D", "B", BUY, 1, 9);
    EXPECT_EQ_OB_UPDATE(updates4[0], "ETHUSD", SELL, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates4[1], "ETHUSD", BUY, 4, 1);
}
