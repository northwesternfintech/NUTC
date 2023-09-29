#include "client_manager/manager.hpp"
#include "lib.hpp"
#include "matching/engine.hpp"
#include "util/messages.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

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
