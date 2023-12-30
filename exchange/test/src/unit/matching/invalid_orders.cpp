#include "config.h"
#include "matching/engine/engine.hpp"
#include "test_utils/macros.hpp"
#include "utils/messages.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitInvalidOrders : public ::testing::Test {
protected:
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        manager_.add_client("ABC", "ABC");
        manager_.add_client("DEF", "DEF");
        manager_.modify_holdings("ABC", "ETHUSD", DEFAULT_QUANTITY);
        manager_.modify_holdings("DEF", "ETHUSD", DEFAULT_QUANTITY);
    }

    ClientManager manager_; // NOLINT (*)
    Engine engine_;         // NOLINT (*)
};

TEST_F(UnitInvalidOrders, SimpleInvalidFunds)
{
    manager_.modify_capital("ABC", -STARTING_CAPITAL);
    std::optional<SIDE> err =
        manager_.validate_match(Match{"ETHUSD", SELL, 1, 1, "ABC", "DEF"});

    if (err.has_value())
        EXPECT_EQ(err.value(), BUY);
    else
        FAIL() << "Match should have failed";
}

TEST_F(UnitInvalidOrders, RemoveThenAddFunds)
{
    manager_.modify_capital("ABC", -STARTING_CAPITAL);

    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};

    // Thrown out
    auto [matches, ob_updates] = engine_.match_order(order1, manager_);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 0);

    // Kept, but not matched
    auto [matches2, ob_updates2] = engine_.match_order(order2, manager_);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2[0], "ETHUSD", SELL, 1, 1);

    manager_.modify_capital("ABC", STARTING_CAPITAL);

    // Kept, but not matched
    auto [matches3, ob_updates3] = engine_.match_order(order2, manager_);
    EXPECT_EQ(matches3.size(), 0);
    EXPECT_EQ(ob_updates3.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates3[0], "ETHUSD", SELL, 1, 1);

    // Kept and matched
    auto [matches4, ob_updates4] = engine_.match_order(order1, manager_);
    EXPECT_EQ(matches4.size(), 1);
    EXPECT_EQ(ob_updates4.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates4[0], "ETHUSD", SELL, 1, 0);
    EXPECT_EQ_MATCH(matches4.at(0), "ETHUSD", "ABC", "DEF", BUY, 1, 1);
}

TEST_F(UnitInvalidOrders, MatchingInvalidFunds)
{
    manager_.modify_capital("ABC", -STARTING_CAPITAL);

    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};

    // Thrown out
    auto [matches, ob_updates] = engine_.match_order(order1, manager_);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 0);

    // Kept, but not matched
    auto [matches2, ob_updates2] = engine_.match_order(order2, manager_);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2[0], "ETHUSD", SELL, 1, 1);
}

TEST_F(UnitInvalidOrders, SimpleManyInvalidOrder)
{
    manager_.add_client("A", "A");
    manager_.add_client("B", "B");
    manager_.add_client("C", "C");
    manager_.add_client("D", "D");
    manager_.modify_capital("B", -STARTING_CAPITAL);
    manager_.modify_holdings("A", "ETHUSD", DEFAULT_QUANTITY);
    manager_.modify_holdings("B", "ETHUSD", DEFAULT_QUANTITY);
    manager_.modify_holdings("C", "ETHUSD", DEFAULT_QUANTITY);
    manager_.modify_holdings("D", "ETHUSD", DEFAULT_QUANTITY);

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
    EXPECT_EQ(updates2.size(), 0);
    EXPECT_EQ(matches3.size(), 0);
    EXPECT_EQ(updates3.size(), 1);

    // Should match two orders and throw out the invalid order (2)
    auto [matches4, updates4] = engine_.match_order(order4, manager_);
    EXPECT_EQ(matches4.size(), 2);
    EXPECT_EQ(updates4.size(), 3);

    EXPECT_EQ_MATCH(matches4[0], "ETHUSD", "A", "D", SELL, 1, 1);
    EXPECT_EQ_MATCH(matches4[1], "ETHUSD", "C", "D", SELL, 1, 1);

    EXPECT_EQ_OB_UPDATE(updates4[0], "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates4[1], "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(updates4[2], "ETHUSD", SELL, 1, 1);
}
