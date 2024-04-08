#include "config.h"
#include "exchange/tickers/engine/engine.hpp"
#include "exchange/traders/trader_manager.hpp"
#include "exchange/traders/trader_types/local_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitBasicMatching : public ::testing::Test {
protected:
    using LocalTrader = nutc::manager::LocalTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        manager_.add_trader<LocalTrader>("ABC", TEST_STARTING_CAPITAL);
        manager_.add_trader<LocalTrader>("DEF", TEST_STARTING_CAPITAL);

        manager_.get_trader("ABC")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("DEF")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    }

    void
    TearDown() override
    {
        manager_.reset();
        SetUp();
    }

    TraderManager& manager_ = nutc::manager::TraderManager::get_instance(); // NOLINT(*)
    Engine engine_{TEST_ORDER_EXPIRATION_TICKS}; // NOLINT (*)

    std::vector<nutc::matching::StoredMatch>
    add_to_engine_(const MarketOrder& order)
    {
        return engine_.match_order(order);
    }
};

TEST_F(UnitBasicMatching, SimpleMatch)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "ABC", "DEF", SELL, 1, 1);
}

TEST_F(UnitBasicMatching, CorrectBuyPricingOrder)
{
    MarketOrder buy1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder buy2{"ABC", BUY, "ETHUSD", 1, 2};
    MarketOrder buy3{"ABC", BUY, "ETHUSD", 1, 3};
    MarketOrder buy4{"ABC", BUY, "ETHUSD", 1, 4};
    MarketOrder sell1{"DEF", SELL, "ETHUSD", 1, 1};

    // Place cheapest buy orders first, then most expensive
    auto matches = add_to_engine_(buy1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(buy3);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(buy2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(buy4);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(sell1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "ABC", "DEF", SELL, 4, 1);
    matches = add_to_engine_(sell1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "ABC", "DEF", SELL, 3, 1);
}

TEST_F(UnitBasicMatching, NoMatchThenMatchBuy)
{
    MarketOrder order1{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", BUY, "ETHUSD", 1, 2};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "DEF", "ABC", BUY, 1, 1);
}

TEST_F(UnitBasicMatching, NoMatchThenMatchSell)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", BUY, "ETHUSD", 1, 1};

    MarketOrder order3{"DEF", SELL, "ETHUSD", 2, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "DEF", "DEF", SELL, 1, 1);
}

TEST_F(UnitBasicMatching, PassivePriceMatch)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 2};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "ABC", "DEF", SELL, 2, 1);
}

TEST_F(UnitBasicMatching, PartialFill)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 2, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
}

TEST_F(UnitBasicMatching, MultipleFill)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", SELL, "ETHUSD", 2, 1};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
}

TEST_F(UnitBasicMatching, MultiplePartialFill)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", SELL, "ETHUSD", 3, 1};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
}

TEST_F(UnitBasicMatching, SimpleMatchReversed)
{
    MarketOrder order1{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", BUY, "ETHUSD", 1, 1};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
}

TEST_F(UnitBasicMatching, PassivePriceMatchReversed)
{
    MarketOrder order1{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", BUY, "ETHUSD", 1, 2};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ(matches.at(0).price, 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
}

TEST_F(UnitBasicMatching, PartialFillReversed)
{
    MarketOrder order1{"ABC", SELL, "ETHUSD", 2, 1};
    MarketOrder order2{"DEF", BUY, "ETHUSD", 1, 1};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
}

TEST_F(UnitBasicMatching, MultipleFillReversed)
{
    MarketOrder order1{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", BUY, "ETHUSD", 2, 1};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
}

TEST_F(UnitBasicMatching, MultiplePartialFillReversed)
{
    MarketOrder order1{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", BUY, "ETHUSD", 3, 1};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
}
