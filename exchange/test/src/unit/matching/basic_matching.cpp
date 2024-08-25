#include "config.h"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitBasicMatching : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;
    TraderContainer traders;

    nutc::traders::GenericTrader& trader1 =
        *traders.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader2 =
        *traders.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader3 =
        *traders.add_trader<TestTrader>(std::string("GHI"), TEST_STARTING_CAPITAL);

    void
    SetUp() override
    {
        trader1.modify_holdings("ETH", DEFAULT_QUANTITY);
        trader2.modify_holdings("ETH", DEFAULT_QUANTITY);
        trader3.modify_holdings("ETH", DEFAULT_QUANTITY);
    }

    void
    TearDown() override
    {
        SetUp();
    }

    nutc::matching::LimitOrderBook orderbook_{};
    Engine engine_;

    std::vector<nutc::messages::match>
    add_to_engine_(const tagged_limit_order& order)
    {
        return engine_.match_order(order, orderbook_);
    }
};

TEST_F(UnitBasicMatching, SimpleMatch)
{
    tagged_limit_order order1{trader1, "ETH", buy, 1, 1.0};
    tagged_limit_order order2{trader2, "ETH", sell, 1, 1.0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETH", "ABC", "DEF", sell, 1, 1);
}

TEST_F(UnitBasicMatching, CorrectBuyPricingOrder)
{
    tagged_limit_order buy1{trader1, "ETH", buy, 1, 1.0};
    tagged_limit_order buy2{trader1, "ETH", buy, 1, 2.0};
    tagged_limit_order buy3{trader1, "ETH", buy, 1, 3.0};
    tagged_limit_order buy4{trader1, "ETH", buy, 1, 4.0};
    tagged_limit_order sell1{trader2, "ETH", sell, 1, 1.0};

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
    ASSERT_EQ_MATCH(matches[0], "ETH", "ABC", "DEF", sell, 1, 4);
    matches = add_to_engine_(sell1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETH", "ABC", "DEF", sell, 1, 3);
}

TEST_F(UnitBasicMatching, NoMatchThenMatchBuy)
{
    tagged_limit_order order1{trader1, "ETH", sell, 1, 1.0};
    tagged_limit_order order2{trader2, "ETH", sell, 1, 1.0};
    tagged_limit_order order3{trader2, "ETH", buy, 1, 2.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETH", "DEF", "ABC", buy, 1, 1);
}

TEST_F(UnitBasicMatching, NoMatchThenMatchSell)
{
    tagged_limit_order order1{trader1, "ETH", buy, 1, 1.0};
    tagged_limit_order order2{trader2, "ETH", buy, 1, 1.0};
    tagged_limit_order order3{trader3, "ETH", sell, 2, 0.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], "ETH", "ABC", "GHI", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETH", "DEF", "GHI", sell, 1, 1);
}

TEST_F(UnitBasicMatching, PassivePriceMatch)
{
    tagged_limit_order order1{trader1, "ETH", buy, 1, 2.0};
    tagged_limit_order order2{trader2, "ETH", sell, 1, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETH", "ABC", "DEF", sell, 1, 2);
}

TEST_F(UnitBasicMatching, PartialFill)
{
    tagged_limit_order order1{trader1, "ETH", buy, 2, 1.0};
    tagged_limit_order order2{trader2, "ETH", sell, 1, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "ABC", "DEF", sell, 1, 1);
}

TEST_F(UnitBasicMatching, MultipleFill)
{
    tagged_limit_order order1{trader1, "ETH", buy, 1, 1.0};
    tagged_limit_order order2{trader1, "ETH", buy, 1, 1.0};
    tagged_limit_order order3{trader2, "ETH", sell, 2, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "ABC", "DEF", sell, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETH", "ABC", "DEF", sell, 1, 1);
}

TEST_F(UnitBasicMatching, MultiplePartialFill)
{
    tagged_limit_order order1{trader1, "ETH", buy, 1, 1.0};
    tagged_limit_order order2{trader1, "ETH", buy, 1, 1.0};
    tagged_limit_order order3{trader2, "ETH", sell, 3, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "ABC", "DEF", sell, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETH", "ABC", "DEF", sell, 1, 1);
}

TEST_F(UnitBasicMatching, SimpleMatchReversed)
{
    tagged_limit_order order1{trader1, "ETH", sell, 1, 1.0};
    tagged_limit_order order2{trader2, "ETH", buy, 1, 1.0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "DEF", "ABC", buy, 1, 1);
}

TEST_F(UnitBasicMatching, PassivePriceMatchReversed)
{
    tagged_limit_order order1{trader1, "ETH", sell, 1, 1.0};
    tagged_limit_order order2{trader2, "ETH", buy, 1, 2.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ(matches.at(0).position.price, 1.0);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "DEF", "ABC", buy, 1, 1);
}

TEST_F(UnitBasicMatching, PartialFillReversed)
{
    tagged_limit_order order1{trader1, "ETH", sell, 2, 1.0};
    tagged_limit_order order2{trader2, "ETH", buy, 1, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "DEF", "ABC", buy, 1, 1);
}

TEST_F(UnitBasicMatching, MultipleFillReversed)
{
    tagged_limit_order order1{trader1, "ETH", sell, 1, 1.0};
    tagged_limit_order order2{trader1, "ETH", sell, 1, 1.0};
    tagged_limit_order order3{trader2, "ETH", buy, 2, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "DEF", "ABC", buy, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETH", "DEF", "ABC", buy, 1, 1);
}

TEST_F(UnitBasicMatching, MultiplePartialFillReversed)
{
    tagged_limit_order order1{trader1, "ETH", sell, 1, 1.0};
    tagged_limit_order order2{trader1, "ETH", sell, 1, 1.0};
    tagged_limit_order order3{trader2, "ETH", buy, 3, 1.0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);

    ASSERT_EQ_MATCH(matches.at(0), "ETH", "DEF", "ABC", buy, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETH", "DEF", "ABC", buy, 1, 1);
}
