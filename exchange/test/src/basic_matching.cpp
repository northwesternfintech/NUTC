#include "client_manager/client_manager.hpp"
#include "lib.hpp"
#include "matching/engine/engine.hpp"
#include "test_utils/macros.hpp"
#include "utils/messages.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class BasicMatching : public ::testing::Test {
protected:
    void
    SetUp() override
    {
        manager.add_client("ABC");
        manager.add_client("DEF");
        manager.modify_holdings("ABC", "ETHUSD", 1000);
        manager.modify_holdings("DEF", "ETHUSD", 1000);
    }

    ClientManager manager;
    Engine engine;
};

TEST_F(BasicMatching, SimpleMatch)
{
    MarketOrder order1{"ABC", BUY,  "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL,  "ETHUSD", 1, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", BUY, 1, 1);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 1);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_MATCH(matches2.at(0), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
}

TEST_F(BasicMatching, CorrectBuyPricingOrder)
{
    MarketOrder buy1{"ABC", BUY,  "ETHUSD", 1, 1};
    MarketOrder buy2{"ABC", BUY,  "ETHUSD", 1, 2};
    MarketOrder buy3{"ABC", BUY,  "ETHUSD", 1, 3};
    MarketOrder buy4{"ABC", BUY,  "ETHUSD", 1, 4};
    MarketOrder sell1{"DEF", SELL,  "ETHUSD", 1, 1};

    // Place cheapest buy orders first, then most expensive
    auto [matches1, ob_updates1] = engine.match_order(buy1, manager);
    auto [matches3, ob_updates3] = engine.match_order(buy3, manager);
    auto [matches2, ob_updates2] = engine.match_order(buy2, manager);
    auto [matches4, ob_updates4] = engine.match_order(buy4, manager);
    EXPECT_EQ(ob_updates1.size(), 1);
    EXPECT_EQ(matches1.size(), 0);
    EXPECT_EQ_OB_UPDATE(ob_updates1.at(0), "ETHUSD", BUY, 1, 1);
    EXPECT_EQ(ob_updates3.size(), 1);
    EXPECT_EQ(matches3.size(), 0);
    EXPECT_EQ_OB_UPDATE(ob_updates3.at(0), "ETHUSD", BUY, 3, 1);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", BUY, 2, 1);
    EXPECT_EQ(ob_updates4.size(), 1);
    EXPECT_EQ(matches4.size(), 0);
    EXPECT_EQ_OB_UPDATE(ob_updates4.at(0), "ETHUSD", BUY, 4, 1);

    auto [matches5, ob_updates5] = engine.match_order(sell1, manager);
    EXPECT_EQ(ob_updates5.size(), 1);
    EXPECT_EQ(matches5.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates5.at(0), "ETHUSD", BUY, 4, 0);

    auto [matches6, ob_updates6] = engine.match_order(sell1, manager);
    EXPECT_EQ(ob_updates6.size(), 1);
    EXPECT_EQ(matches6.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates6.at(0), "ETHUSD", BUY, 3, 0);
}

TEST_F(BasicMatching, NoMatchThenMatchBuy)
{
    MarketOrder order1{"ABC", SELL,  "ETHUSD", 1, 1000};
    MarketOrder order2{"DEF", SELL,  "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", BUY,  "ETHUSD", 1, 2};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    auto [matches3, ob_updates3] = engine.match_order(order3, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(matches3.size(), 1);
}

TEST_F(BasicMatching, NoMatchThenMatchSell)
{
    MarketOrder order1{"ABC", BUY,  "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", BUY,  "ETHUSD", 1, 1000};

    MarketOrder order3{"DEF", SELL,  "ETHUSD", 1, 500};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    auto [matches3, ob_updates3] = engine.match_order(order1, manager);
    auto [matches4, ob_updates4] = engine.match_order(order3, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(matches3.size(), 0);
    EXPECT_EQ(matches4.size(), 1);
}

TEST_F(BasicMatching, PassivePriceMatch)
{
    MarketOrder order1{"ABC", BUY,  "ETHUSD", 1, 2};
    MarketOrder order2{"DEF", SELL,  "ETHUSD", 1, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", BUY, 2, 1);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 1);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", BUY, 2, 0);
    EXPECT_EQ_MATCH(matches2.at(0), "ETHUSD", "ABC", "DEF", SELL, 2, 1);
}

TEST_F(BasicMatching, PartialFill)
{
    MarketOrder order1{"ABC", BUY,  "ETHUSD", 2, 1};
    MarketOrder order2{"DEF", SELL,  "ETHUSD", 1, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", BUY, 1, 2);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 1);
    EXPECT_EQ(ob_updates2.size(), 2);
    EXPECT_EQ_MATCH(matches2.at(0), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(1), "ETHUSD", BUY, 1, 1);
}

TEST_F(BasicMatching, MultipleFill)
{
    MarketOrder order1{"ABC", BUY,  "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", BUY,  "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", SELL,  "ETHUSD", 2, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", BUY, 1, 1);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", BUY, 1, 1);

    auto [matches3, ob_updates3] = engine.match_order(order3, manager);
    EXPECT_EQ(matches3.size(), 2);
    EXPECT_EQ(ob_updates3.size(), 2);
    EXPECT_EQ_MATCH(matches3.at(0), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    EXPECT_EQ_MATCH(matches3.at(1), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    EXPECT_EQ_OB_UPDATE(ob_updates3.at(0), "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(ob_updates3.at(1), "ETHUSD", BUY, 1, 0);
}

TEST_F(BasicMatching, MultiplePartialFill)
{
    MarketOrder order1{"ABC", BUY,  "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", BUY,  "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", SELL,  "ETHUSD", 3, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", BUY, 1, 1);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", BUY, 1, 1);

    auto [matches3, ob_updates3] = engine.match_order(order3, manager);
    EXPECT_EQ(matches3.size(), 2);
    EXPECT_EQ(ob_updates3.size(), 3);
    EXPECT_EQ_MATCH(matches3.at(0), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    EXPECT_EQ_MATCH(matches3.at(1), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    EXPECT_EQ_OB_UPDATE(ob_updates3.at(0), "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(ob_updates3.at(1), "ETHUSD", BUY, 1, 0);
    EXPECT_EQ_OB_UPDATE(ob_updates3.at(2), "ETHUSD", SELL, 1, 1);
}

TEST_F(BasicMatching, SimpleMatchReversed)
{
    MarketOrder order1{"ABC", SELL,  "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", BUY,  "ETHUSD", 1, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", SELL, 1, 1);
    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 1);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", SELL, 1, 0);
    EXPECT_EQ_MATCH(matches2.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
}

TEST_F(BasicMatching, PassivePriceMatchReversed)
{
    MarketOrder order1{"ABC", SELL,  "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", BUY,  "ETHUSD", 1, 2};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", SELL, 1, 1);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 1);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ(matches2.at(0).price, 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", SELL, 1, 0);
    EXPECT_EQ_MATCH(matches2.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
}

TEST_F(BasicMatching, PartialFillReversed)
{
    MarketOrder order1{"ABC", SELL,  "ETHUSD", 2, 1};
    MarketOrder order2{"DEF", BUY,  "ETHUSD", 1, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", SELL, 1, 2);
    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 1);
    EXPECT_EQ(ob_updates2.size(), 2);
    EXPECT_EQ_MATCH(matches2.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", SELL, 1, 0);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(1), "ETHUSD", SELL, 1, 1);
}

TEST_F(BasicMatching, MultipleFillReversed)
{
    MarketOrder order1{"ABC", SELL,  "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", SELL,  "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", BUY,  "ETHUSD", 2, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", SELL, 1, 1);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", SELL, 1, 1);

    auto [matches3, ob_updates3] = engine.match_order(order3, manager);
    EXPECT_EQ(matches3.size(), 2);
    EXPECT_EQ(ob_updates3.size(), 2);
    EXPECT_EQ_MATCH(matches3.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
    EXPECT_EQ_MATCH(matches3.at(1), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
    EXPECT_EQ_OB_UPDATE(ob_updates3.at(0), "ETHUSD", SELL, 1, 0);
    EXPECT_EQ_OB_UPDATE(ob_updates3.at(1), "ETHUSD", SELL, 1, 0);
}

TEST_F(BasicMatching, MultiplePartialFillReversed)
{
    MarketOrder order1{"ABC", SELL,  "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", SELL,  "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", BUY,  "ETHUSD", 3, 1};
    auto [matches, ob_updates] = engine.match_order(order1, manager);
    EXPECT_EQ(matches.size(), 0);
    EXPECT_EQ(ob_updates.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", SELL, 1, 1);

    auto [matches2, ob_updates2] = engine.match_order(order2, manager);
    EXPECT_EQ(matches2.size(), 0);
    EXPECT_EQ(ob_updates2.size(), 1);
    EXPECT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", SELL, 1, 1);

    auto [matches3, ob_updates3] = engine.match_order(order3, manager);
    EXPECT_EQ(matches3.size(), 2);
    EXPECT_EQ(ob_updates3.size(), 3);
    EXPECT_EQ_MATCH(matches3.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
    EXPECT_EQ_MATCH(matches3.at(1), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
    EXPECT_EQ_OB_UPDATE(ob_updates3.at(0), "ETHUSD", SELL, 1, 0);
    EXPECT_EQ_OB_UPDATE(ob_updates3.at(1), "ETHUSD", SELL, 1, 0);
    EXPECT_EQ_OB_UPDATE(ob_updates3.at(2), "ETHUSD", BUY, 1, 1);
}
