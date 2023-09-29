#include "client_manager/manager.hpp"
#include "lib.hpp"
#include "matching/engine.hpp"
#include "util/macros.hpp"
#include "util/messages.hpp"

#include <gtest/gtest.h>

using nutc::messages::BUY;
using nutc::messages::SELL;

class InvalidOrders : public ::testing::Test {
protected:
    void
    SetUp() override
    {
        manager.addClient("ABC");
        manager.addClient("DEF");
    }

    ClientManager manager;
    Engine engine;
};

TEST_F(InvalidOrders, SimpleInvalidFunds)
{
    manager.modifyCapital("ABC", -1000);
    std::optional<SIDE> err =
        manager.validateMatch(Match{"ETHUSD", "ABC", "DEF", nutc::messages::SELL, 1, 1}
        );
    EXPECT_TRUE(err.has_value());
    EXPECT_EQ(err.value(), nutc::messages::BUY);
}

TEST_F(InvalidOrders, RemoveThenAddFunds)
{
    manager.modifyCapital("ABC", -1000);

    MarketOrder order1{"ABC", nutc::messages::BUY, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", nutc::messages::SELL, "MARKET", "ETHUSD", 1, 1};

    // Thrown out
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 0);

    // Kept, but not matched
    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(ob_updates2.size(), 1);

    manager.modifyCapital("ABC", 1000);

    // Kept, but not matched
    auto [matches3, ob_updates3] = engine.match_order(order2, manager);
    // EXPECT_EQ(matches3.size(), 0);
    // EXPECT_EQ(ob_updates3.size(), 1);

    // Kept and matched
    auto [matches4, ob_updates4] = engine.match_order(order1, manager);
    // EXPECT_EQ(matches4.size(), 1);
    // EXPECT_EQ(ob_updates4.size(), 1);
}

TEST_F(InvalidOrders, MatchingInvalidFunds)
{
    manager.modifyCapital("ABC", -1000);

    MarketOrder order1{"ABC", nutc::messages::BUY, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", nutc::messages::SELL, "MARKET", "ETHUSD", 1, 1};

    // Thrown out
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 0);

    // Kept, but not matched
    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(ob_updates2.size(), 1);
}

TEST_F(InvalidOrders, SimpleManyOrder)
{
    manager.addClient("A");
    manager.addClient("B");
    manager.addClient("C");
    manager.addClient("D");
    manager.modifyCapital("B", -1000);
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
    EXPECT_EQ(updates2.size(), 0);
    EXPECT_EQ(matches3.size(), 0);
    EXPECT_EQ(updates3.size(), 1);

    // Should match two orders and throw out the invalid order (2)
    auto [matches4, updates4] = engine.match_order(order4, manager);
    EXPECT_EQ(matches4.size(), 2);
    EXPECT_EQ(updates4.size(), 3);

    EXPECT_EQ_MATCH(matches4[0], "ETHUSD", "A", "D", SELL, 1, 1);
    EXPECT_EQ_MATCH(matches4[1], "ETHUSD", "C", "D", SELL, 1, 1);

    EXPECT_EQ_OB_UPDATE(updates4[0], "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates4[1], "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates4[2], "ETHUSD", SELL, 1, 1);
}
