#include "config.h"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

using nutc::shared::Ticker;
using nutc::shared::Side::buy;
using nutc::shared::Side::sell;

class UnitOrderFeeMatching : public ::testing::Test {
protected:
    using TestTrader = nutc::test::TestTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;
    TraderContainer traders;
    nutc::exchange::GenericTrader& trader1 =
        *traders.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
    nutc::exchange::GenericTrader& trader2 =
        *traders.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);
    nutc::exchange::GenericTrader& trader3 =
        *traders.add_trader<TestTrader>(std::string("GHI"), TEST_STARTING_CAPITAL);

    void
    SetUp() override
    {
        trader1.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader2.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader3.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    }

    void
    TearDown() override
    {
        SetUp();
    }

    nutc::exchange::LimitOrderBook orderbook_;
    Engine engine_{.5};

    std::vector<nutc::shared::match>
    add_to_engine_(const tagged_limit_order& order)
    {
        return engine_.match_order(order, orderbook_);
    }
};

TEST_F(UnitOrderFeeMatching, SimpleMatch)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0, 0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1, 1.0, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "DEF", sell, 1, 1);

    ASSERT_EQ(trader1.get_capital_delta(), -1.5);
    ASSERT_EQ(trader2.get_capital_delta(), .5);
}

TEST_F(UnitOrderFeeMatching, MultipleMatches)
{
    tagged_limit_order buy1{trader1, Ticker::ETH, buy, 1, 1.0, 0};
    tagged_limit_order buy2{trader1, Ticker::ETH, buy, 1, 2.0, 0};
    tagged_limit_order buy3{trader1, Ticker::ETH, buy, 1, 3.0, 0};
    tagged_limit_order buy4{trader1, Ticker::ETH, buy, 1, 4.0, 0};
    tagged_limit_order sell1{trader2, Ticker::ETH, sell, 1, 1.0, 0};

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
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "DEF", sell, 1, 4);

    ASSERT_EQ(trader1.get_capital_delta(), -4 * 1.5);
    ASSERT_EQ(trader2.get_capital_delta(), 4 * .5);

    matches = add_to_engine_(sell1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "DEF", sell, 1, 3);

    ASSERT_EQ(trader1.get_capital_delta(), -4 * 1.5 + -3 * 1.5);
    ASSERT_EQ(trader2.get_capital_delta(), 4 * .5 + 3 * .5);
}

TEST_F(UnitOrderFeeMatching, NoMatchThenMatchBuy)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1, 1.0, 0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1, 1.0, 0};
    tagged_limit_order order3{trader2, Ticker::ETH, buy, 1, 2.0, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "DEF", "ABC", buy, 1, 1);

    ASSERT_EQ(trader1.get_capital_delta(), .5);
    ASSERT_EQ(trader2.get_capital_delta(), -1.5);
}

TEST_F(UnitOrderFeeMatching, NoMatchThenMatchSell)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0, 0};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1, 1.0, 0};
    tagged_limit_order order3{trader3, Ticker::ETH, sell, 2, 0.0, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "GHI", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], Ticker::ETH, "DEF", "GHI", sell, 1, 1);

    ASSERT_EQ(trader1.get_capital_delta(), -1.5);
    ASSERT_EQ(trader2.get_capital_delta(), -1.5);
    ASSERT_EQ(trader3.get_capital_delta(), 1);
}

TEST_F(UnitOrderFeeMatching, PassivePriceMatchWithVolume)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 2, 2.0, 0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 2, 1.0, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "DEF", sell, 2, 2);

    ASSERT_EQ(trader1.get_capital_delta(), -2 * 2 * 1.5);
    ASSERT_EQ(trader2.get_capital_delta(), 2 * 2 * .5);
}

TEST_F(UnitOrderFeeMatching, PartialFill)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 2, 1.0, 0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1, 1.0, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "ABC", "DEF", sell, 1, 1);

    ASSERT_EQ(trader1.get_capital_delta(), -1 * 1.5);
    ASSERT_EQ(trader2.get_capital_delta(), 1 * .5);
}

TEST_F(UnitOrderFeeMatching, MultipleFill)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0, 0};
    tagged_limit_order order2{trader1, Ticker::ETH, buy, 1, 1.0, 0};
    tagged_limit_order order3{trader2, Ticker::ETH, sell, 2, 1.0, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "ABC", "DEF", sell, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), Ticker::ETH, "ABC", "DEF", sell, 1, 1);

    ASSERT_EQ(trader1.get_capital_delta(), -2 * 1 * 1.5);
    ASSERT_EQ(trader2.get_capital_delta(), 2 * .5);
}

TEST_F(UnitOrderFeeMatching, MultiplePartialFill)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0, 0};
    tagged_limit_order order2{trader1, Ticker::ETH, buy, 1, 1.0, 0};
    tagged_limit_order order3{trader2, Ticker::ETH, sell, 3, 1.0, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "ABC", "DEF", sell, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), Ticker::ETH, "ABC", "DEF", sell, 1, 1);

    ASSERT_EQ(trader1.get_capital_delta(), -2 * 1 * 1.5);
    ASSERT_EQ(trader2.get_capital_delta(), 2 * .5);
}

TEST_F(UnitOrderFeeMatching, SimpleMatchReversed)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1, 1.0, 0};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1, 1.0, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "DEF", "ABC", buy, 1, 1);

    ASSERT_EQ(trader2.get_capital_delta(), -1 * 1 * 1.5);
    ASSERT_EQ(trader1.get_capital_delta(), 1 * .5);
}

TEST_F(UnitOrderFeeMatching, PassivePriceMatchReversed)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1, 1.0, 0};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1, 2.0, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ(matches.at(0).position.price, 1.0);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "DEF", "ABC", buy, 1, 1);
    ASSERT_EQ(trader2.get_capital_delta(), -1 * 1 * 1.5);
    ASSERT_EQ(trader1.get_capital_delta(), 1 * .5);
}

TEST_F(UnitOrderFeeMatching, PartialFillReversed)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 2, 1.0, 0};
    tagged_limit_order order2{trader2, Ticker::ETH, buy, 1, 1.0, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "DEF", "ABC", buy, 1, 1);
    ASSERT_EQ(trader2.get_capital_delta(), -1 * 1 * 1.5);
    ASSERT_EQ(trader1.get_capital_delta(), 1 * .5);
}

TEST_F(UnitOrderFeeMatching, MultipleFillReversed)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1, 1.0, 0};
    tagged_limit_order order2{trader1, Ticker::ETH, sell, 1, 1.0, 0};
    tagged_limit_order order3{trader2, Ticker::ETH, buy, 2, 1.0, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "DEF", "ABC", buy, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), Ticker::ETH, "DEF", "ABC", buy, 1, 1);

    ASSERT_EQ(trader2.get_capital_delta(), -2 * 1 * 1.5);
    ASSERT_EQ(trader1.get_capital_delta(), 2 * .5);
}

TEST_F(UnitOrderFeeMatching, MultiplePartialFillReversed)
{
    tagged_limit_order order1{trader1, Ticker::ETH, sell, 1, 1.0, 0};
    tagged_limit_order order2{trader1, Ticker::ETH, sell, 1, 1.0, 0};
    tagged_limit_order order3{trader2, Ticker::ETH, buy, 3, 1.0, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), Ticker::ETH, "DEF", "ABC", buy, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), Ticker::ETH, "DEF", "ABC", buy, 1, 1);

    ASSERT_EQ(trader2.get_capital_delta(), -2 * 1 * 1.5);
    ASSERT_EQ(trader1.get_capital_delta(), 2 * .5);
}

TEST_F(UnitOrderFeeMatching, NotEnoughToEnough)
{
    trader1.modify_capital(-TEST_STARTING_CAPITAL + 1);

    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1, 1.0, 0};
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0, 0};

    // Thrown out
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    // Kept, but not matched
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    ASSERT_EQ(trader1.get_capital(), 1);
    ASSERT_EQ(trader2.get_capital_delta(), 0);

    trader1.modify_capital(0.5);
    ;

    // Kept, but not matched
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    // Kept and matched
    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "ABC", "DEF", buy, 1, 1);

    ASSERT_EQ(trader1.get_capital(), 0);
    ASSERT_EQ(trader2.get_capital_delta(), 1 * .5);
}

TEST_F(UnitOrderFeeMatching, MatchingInvalidFunds)
{
    trader1.modify_capital(-TEST_STARTING_CAPITAL + 1);

    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0, 0};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1, 1.0, 0};

    // Thrown out
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    // Kept, but not matched
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    ASSERT_EQ(trader1.get_capital(), 1);
    ASSERT_EQ(trader2.get_capital_delta(), 0);
}

TEST_F(UnitOrderFeeMatching, SimpleManyInvalidOrder)
{
    nutc::exchange::GenericTrader& trader4 =
        *traders.add_trader<TestTrader>(std::string("A"), TEST_STARTING_CAPITAL);
    nutc::exchange::GenericTrader& trader5 =
        *traders.add_trader<TestTrader>(std::string("B"), 1);
    nutc::exchange::GenericTrader& trader6 =
        *traders.add_trader<TestTrader>(std::string("C"), TEST_STARTING_CAPITAL);
    nutc::exchange::GenericTrader& trader7 =
        *traders.add_trader<TestTrader>(std::string("D"), TEST_STARTING_CAPITAL);

    trader4.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    trader5.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    trader6.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    trader7.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);

    tagged_limit_order order1{trader4, Ticker::ETH, buy, 1, 1.0, 0};
    tagged_limit_order order2{trader5, Ticker::ETH, buy, 1, 1.0, 0};
    tagged_limit_order order3{trader6, Ticker::ETH, buy, 1, 1.0, 0};
    tagged_limit_order order4{trader7, Ticker::ETH, sell, 3, 1.0, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 0);

    // Should match two orders and throw out the invalid order (2)
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 2);

    ASSERT_EQ_MATCH(matches[0], Ticker::ETH, "A", "D", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], Ticker::ETH, "C", "D", sell, 1, 1);

    ASSERT_EQ(trader4.get_capital_delta(), -1.5);
    ASSERT_EQ(trader5.get_capital_delta(), 0);
    ASSERT_EQ(trader6.get_capital_delta(), -1 * 1 * 1.5);
    ASSERT_EQ(trader7.get_capital_delta(), 2 * .5);
}
