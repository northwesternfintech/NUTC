#include "client_manager/manager.hpp"
#include "lib.hpp"
#include "matching/engine.hpp"
#include "util/messages.hpp"

#include <gtest/gtest.h>
using Engine = nutc::matching::Engine;
using MarketOrder = nutc::messages::MarketOrder;
using ClientManager = nutc::manager::ClientManager;

class BasicMatching : public ::testing::Test {
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

TEST_F(BasicMatching, SimpleMatch)
{
    MarketOrder order1{"ABC", nutc::messages::BUY, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", nutc::messages::SELL, "MARKET", "ETHUSD", 1, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 1);
    EXPECT_EQ(ob_updates2.size(), 1);
}

TEST_F(BasicMatching, PassivePriceMatch)
{
    MarketOrder order1{"ABC", nutc::messages::BUY, "MARKET", "ETHUSD", 1, 2};
    MarketOrder order2{"DEF", nutc::messages::SELL, "MARKET", "ETHUSD", 1, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 1);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ(matches2.at(0).price, 2);
}

TEST_F(BasicMatching, PartialFill)
{
    MarketOrder order1{"ABC", nutc::messages::BUY, "MARKET", "ETHUSD", 2, 1};
    MarketOrder order2{"DEF", nutc::messages::SELL, "MARKET", "ETHUSD", 1, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 1);
    EXPECT_EQ(ob_updates2.size(), 2);
}

TEST_F(BasicMatching, MultipleFill)
{
    MarketOrder order1{"ABC", nutc::messages::BUY, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", nutc::messages::BUY, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", nutc::messages::SELL, "MARKET", "ETHUSD", 2, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(ob_updates2.size(), 1);

    auto [matches3, ob_updates3] = engine.match_order(order3, manager);
    EXPECT_EQ(matches3.size(), 2);
    EXPECT_EQ(ob_updates3.size(), 2);
}

TEST_F(BasicMatching, MultiplePartialFill)
{
    MarketOrder order1{"ABC", nutc::messages::BUY, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", nutc::messages::BUY, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", nutc::messages::SELL, "MARKET", "ETHUSD", 3, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(ob_updates2.size(), 1);

    auto [matches3, ob_updates3] = engine.match_order(order3, manager);
    EXPECT_EQ(matches3.size(), 2);
    EXPECT_EQ(ob_updates3.size(), 3);
}

