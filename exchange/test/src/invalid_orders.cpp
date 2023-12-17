#include "client_manager/client_manager.hpp"
#include "matching/engine/engine.hpp"
#include "test_utils/macros.hpp"
#include "utils/messages.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class InvalidOrders : public ::testing::Test {
protected:
    void
    SetUp() override
    {
        manager.add_client("ABC", "ABC");
        manager.add_client("DEF", "DEF");
        manager.modify_holdings("ABC", "ETHUSD", 1000);
        manager.modify_holdings("DEF", "ETHUSD", 1000);
    }

    ClientManager manager;
    Engine engine;
};

TEST_F(InvalidOrders, SimpleInvalidFunds)
{
    manager.modify_capital("ABC", -100000);
    std::optional<SIDE> err =
        manager.validate_match(Match{"ETHUSD", SELL, 1, 1, "ABC", "DEF"});
    EXPECT_TRUE(err.has_value());
    EXPECT_EQ(err.value(), BUY);
}

TEST_F(InvalidOrders, RemoveThenAddFunds)
{
    manager.modify_capital("ABC", -100000);

    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};

    // Thrown out
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 0);

    // Kept, but not matched
    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2[0], "ETHUSD", SELL, 1, 1);

    manager.modify_capital("ABC", 100000);

    // Kept, but not matched
    auto [matches3, ob_updates3] = engine.match_order(order2, manager);
    EXPECT_EQ(matches3.size(), 0);
    EXPECT_EQ(ob_updates3.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates3[0], "ETHUSD", SELL, 1, 1);

    // Kept and matched
    auto [matches4, ob_updates4] = engine.match_order(order1, manager);
    EXPECT_EQ(matches4.size(), 1);
    EXPECT_EQ(ob_updates4.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates4[0], "ETHUSD", SELL, 1, 0);
    EXPECT_EQ_MATCH(matches4.at(0), "ETHUSD", "ABC", "DEF", BUY, 1, 1);
}

// TODO: valid when first added, then invalid due to reducing capital

TEST_F(InvalidOrders, MatchingInvalidFunds)
{
    manager.modify_capital("ABC", -100000);

    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};

    // Thrown out
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 0);

    // Kept, but not matched
    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2[0], "ETHUSD", SELL, 1, 1);
}

TEST_F(InvalidOrders, SimpleManyInvalidOrder)
{
    manager.add_client("A", "A");
    manager.add_client("B", "B");
    manager.add_client("C", "C");
    manager.add_client("D", "D");
    manager.modify_capital("B", -100000);
    manager.modify_holdings("A", "ETHUSD", 1000);
    manager.modify_holdings("B", "ETHUSD", 1000);
    manager.modify_holdings("C", "ETHUSD", 1000);
    manager.modify_holdings("D", "ETHUSD", 1000);

    MarketOrder order1{"A", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"B", BUY, "ETHUSD", 1, 1};
    MarketOrder order3{"C", BUY, "ETHUSD", 1, 1};
    MarketOrder order4{"D", SELL, "ETHUSD", 3, 1};

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
