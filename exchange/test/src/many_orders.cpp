#include "util/macros.hpp"
#include "util/messages.hpp"

#include <gtest/gtest.h>

using nutc::messages::BUY;
using nutc::messages::SELL;

class ManyOrders : public ::testing::Test {
protected:
    void
    SetUp() override
    {
        manager.addClient("A");
        manager.addClient("B");
        manager.addClient("C");
        manager.addClient("D");
        manager.modifyHoldings("A", "ETHUSD", 1000);
        manager.modifyHoldings("B", "ETHUSD", 1000);
        manager.modifyHoldings("C", "ETHUSD", 1000);
        manager.modifyHoldings("D", "ETHUSD", 1000);
    }

    ClientManager manager;
    Engine engine;
};

TEST_F(ManyOrders, OnlyMatchesOne)
{
    MarketOrder order1{"A", BUY, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order2{"B", SELL, "MARKET", "ETHUSD", 1, 1};

    auto [matches1, updates1] = engine.match_order(order1, manager);
    auto [matches2, updates2] = engine.match_order(order1, manager);
    EXPECT_EQ(matches1.size(), 0);
    EXPECT_EQ(updates1.size(), 1);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(updates2.size(), 1);

    auto [matches3, updates3] = engine.match_order(order2, manager);
    EXPECT_EQ(matches3.size(), 1);
    EXPECT_EQ(updates3.size(), 1);
    EXPECT_EQ_MATCH(matches3[0], "ETHUSD", "A", "B", SELL, 1, 1);
    EXPECT_EQ_OB_UPDATE(updates3[0], "ETHUSD", BUY, 1, 0);
}

TEST_F(ManyOrders, SimpleManyOrder)
{
    MarketOrder order1{"A", BUY, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order2{"B", BUY, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order3{"C", BUY, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order4{"D", SELL, "MARKET", "ETHUSD", 3, 1};

    auto [matches1, updates1] = engine.match_order(order1, manager);
    auto [matches2, updates2] = engine.match_order(order2, manager);
    auto [matches3, updates3] = engine.match_order(order3, manager);

    EXPECT_EQ(matches1.size(), 0);
    EXPECT_EQ(updates1.size(), 1);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(updates2.size(), 1);
    EXPECT_EQ(matches3.size(), 0);
    EXPECT_EQ(updates3.size(), 1);

    auto [matches4, updates4] = engine.match_order(order4, manager);
    EXPECT_EQ(matches4.size(), 3);
    EXPECT_EQ(updates4.size(), 3);

    EXPECT_EQ_MATCH(matches4[0], "ETHUSD", "A", "D", SELL, 1, 1);
    EXPECT_EQ_MATCH(matches4[1], "ETHUSD", "B", "D", SELL, 1, 1);
    EXPECT_EQ_MATCH(matches4[2], "ETHUSD", "C", "D", SELL, 1, 1);

    EXPECT_EQ_OB_UPDATE(updates4[0], "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates4[1], "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates4[2], "ETHUSD", BUY, 1, 0);
}

TEST_F(ManyOrders, PassiveAndAggressivePartial)
{
    MarketOrder order1{"A", SELL, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order2{"B", SELL, "MARKET", "ETHUSD", 10, 1};
    MarketOrder order3{"C", BUY, "MARKET", "ETHUSD", 2, 3};
    MarketOrder order4{"D", BUY, "MARKET", "ETHUSD", 10, 4};

    auto [matches1, updates1] = engine.match_order(order1, manager);
    auto [matches2, updates2] = engine.match_order(order2, manager);
    auto [matches3, updates3] = engine.match_order(order3, manager);
    auto [matches4, updates4] = engine.match_order(order4, manager);

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
