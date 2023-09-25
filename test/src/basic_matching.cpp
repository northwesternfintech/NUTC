#include "lib.hpp"
#include "matching/engine.hpp"
#include "util/messages.hpp"

#include <gtest/gtest.h>
using Engine = nutc::matching::Engine;
using MarketOrder = nutc::messages::MarketOrder;

TEST(BasicMatching, SimpleMatch)
{
    Engine engine;
    MarketOrder order1{"ABC", nutc::messages::BUY, "MARKET", "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", nutc::messages::SELL, "MARKET", "ETHUSD", 1, 1};
    auto [matches, ob_updates] = engine.match_order(order1);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);

    auto [matches2, ob_updates2] = engine.match_order(order2);
    EXPECT_EQ(matches2.size(), 1);
    EXPECT_EQ(ob_updates2.size(), 1);
}

TEST(BasicMatching, PassivePriceMatch)
{
    Engine engine;
    MarketOrder order1{"ABC", nutc::messages::BUY, "MARKET", "ETHUSD", 1, 2};
    MarketOrder order2{"DEF", nutc::messages::SELL, "MARKET", "ETHUSD", 1, 1};
    auto [matches, ob_updates] = engine.match_order(order1);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);

    auto [matches2, ob_updates2] = engine.match_order(order2);
    EXPECT_EQ(matches2.size(), 1);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ(matches2.at(0).price, 2);
}

TEST(BasicMatching, PartialFill)
{
    Engine engine;
    MarketOrder order1{"ABC", nutc::messages::BUY, "MARKET", "ETHUSD", 2, 1};
    MarketOrder order2{"DEF", nutc::messages::SELL, "MARKET", "ETHUSD", 1, 1};
    auto [matches, ob_updates] = engine.match_order(order1);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);

    auto [matches2, ob_updates2] = engine.match_order(order2);
    EXPECT_EQ(matches2.size(), 1);
    EXPECT_EQ(ob_updates2.size(), 2);
}
