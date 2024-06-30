#include "config.h"
#include "exchange/orders/storage/orderbook.hpp"
#include "exchange/traders/trader_container.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitOrderFeeMatching : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;
    TraderContainer& manager_ = nutc::traders::TraderContainer::get_instance();
    nutc::traders::GenericTrader& trader1 =
        *manager_.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader2 =
        *manager_.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader3 =
        *manager_.add_trader<TestTrader>(std::string("GHI"), TEST_STARTING_CAPITAL);

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

    nutc::matching::OrderBook orderbook_;
    Engine engine_{.5};

    std::vector<nutc::matching::stored_match>
    add_to_engine_(const stored_order& order)
    {
        return engine_.match_order(orderbook_, order);
    }
};

TEST_F(UnitOrderFeeMatching, SimpleMatch)
{
    stored_order order1{trader1, "ETH", buy, 1, 1, 0};
    stored_order order2{trader2, "ETH", sell, 1, 1, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETH", "ABC", "DEF", sell, 1, 1);

    ASSERT_EQ(trader1.get_capital_delta(), -1.5);
    ASSERT_EQ(trader2.get_capital_delta(), .5);
}

TEST_F(UnitOrderFeeMatching, MultipleMatches)
{
    stored_order buy1{trader1, "ETH", buy, 1, 1, 0};
    stored_order buy2{trader1, "ETH", buy, 2, 1, 0};
    stored_order buy3{trader1, "ETH", buy, 3, 1, 0};
    stored_order buy4{trader1, "ETH", buy, 4, 1, 0};
    stored_order sell1{trader2, "ETH", sell, 1, 1, 0};

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
    ASSERT_EQ_MATCH(matches[0], "ETH", "ABC", "DEF", sell, 4, 1);

    ASSERT_EQ(trader1.get_capital_delta(), -4 * 1.5);
    ASSERT_EQ(trader2.get_capital_delta(), 4 * .5);

    matches = add_to_engine_(sell1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETH", "ABC", "DEF", sell, 3, 1);

    ASSERT_EQ(trader1.get_capital_delta(), -4 * 1.5 + -3 * 1.5);
    ASSERT_EQ(trader2.get_capital_delta(), 4 * .5 + 3 * .5);
}

TEST_F(UnitOrderFeeMatching, NoMatchThenMatchBuy)
{
    stored_order order1{trader1, "ETH", sell, 1, 1, 0};
    stored_order order2{trader2, "ETH", sell, 1, 1, 0};
    stored_order order3{trader2, "ETH", buy, 2, 1, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETH", "DEF", "ABC", buy, 1, 1);

    ASSERT_EQ(trader1.get_capital_delta(), .5);
    ASSERT_EQ(trader2.get_capital_delta(), -1.5);
}

TEST_F(UnitOrderFeeMatching, NoMatchThenMatchSell)
{
    stored_order order1{trader1, "ETH", buy, 1, 1, 0};
    stored_order order2{trader2, "ETH", buy, 1, 1, 0};
    stored_order order3{trader3, "ETH", sell, 0, 2, 0};

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

    ASSERT_EQ(trader1.get_capital_delta(), -1.5);
    ASSERT_EQ(trader2.get_capital_delta(), -1.5);
    ASSERT_EQ(trader3.get_capital_delta(), 1);
}

TEST_F(UnitOrderFeeMatching, PassivePriceMatchWithVolume)
{
    stored_order order1{trader1, "ETH", buy, 2, 2, 0};
    stored_order order2{trader2, "ETH", sell, 1, 2, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETH", "ABC", "DEF", sell, 2, 2);

    ASSERT_EQ(trader1.get_capital_delta(), -2 * 2 * 1.5);
    ASSERT_EQ(trader2.get_capital_delta(), 2 * 2 * .5);
}

TEST_F(UnitOrderFeeMatching, PartialFill)
{
    stored_order order1{trader1, "ETH", buy, 1, 2, 0};
    stored_order order2{trader2, "ETH", sell, 1, 1, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "ABC", "DEF", sell, 1, 1);

    ASSERT_EQ(trader1.get_capital_delta(), -1 * 1.5);
    ASSERT_EQ(trader2.get_capital_delta(), 1 * .5);
}

TEST_F(UnitOrderFeeMatching, MultipleFill)
{
    stored_order order1{trader1, "ETH", buy, 1, 1, 0};
    stored_order order2{trader1, "ETH", buy, 1, 1, 0};
    stored_order order3{trader2, "ETH", sell, 1, 2, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "ABC", "DEF", sell, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETH", "ABC", "DEF", sell, 1, 1);

    ASSERT_EQ(trader1.get_capital_delta(), -2 * 1 * 1.5);
    ASSERT_EQ(trader2.get_capital_delta(), 2 * .5);
}

TEST_F(UnitOrderFeeMatching, MultiplePartialFill)
{
    stored_order order1{trader1, "ETH", buy, 1, 1, 0};
    stored_order order2{trader1, "ETH", buy, 1, 1, 0};
    stored_order order3{trader2, "ETH", sell, 1, 3, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "ABC", "DEF", sell, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETH", "ABC", "DEF", sell, 1, 1);

    ASSERT_EQ(trader1.get_capital_delta(), -2 * 1 * 1.5);
    ASSERT_EQ(trader2.get_capital_delta(), 2 * .5);
}

TEST_F(UnitOrderFeeMatching, SimpleMatchReversed)
{
    stored_order order1{trader1, "ETH", sell, 1, 1, 0};
    stored_order order2{trader2, "ETH", buy, 1, 1, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "DEF", "ABC", buy, 1, 1);

    ASSERT_EQ(trader2.get_capital_delta(), -1 * 1 * 1.5);
    ASSERT_EQ(trader1.get_capital_delta(), 1 * .5);
}

TEST_F(UnitOrderFeeMatching, PassivePriceMatchReversed)
{
    stored_order order1{trader1, "ETH", sell, 1, 1, 0};
    stored_order order2{trader2, "ETH", buy, 2, 1, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ(matches.at(0).price, 1.0);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "DEF", "ABC", buy, 1, 1);
    ASSERT_EQ(trader2.get_capital_delta(), -1 * 1 * 1.5);
    ASSERT_EQ(trader1.get_capital_delta(), 1 * .5);
}

TEST_F(UnitOrderFeeMatching, PartialFillReversed)
{
    stored_order order1{trader1, "ETH", sell, 1, 2, 0};
    stored_order order2{trader2, "ETH", buy, 1, 1, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "DEF", "ABC", buy, 1, 1);
    ASSERT_EQ(trader2.get_capital_delta(), -1 * 1 * 1.5);
    ASSERT_EQ(trader1.get_capital_delta(), 1 * .5);
}

TEST_F(UnitOrderFeeMatching, MultipleFillReversed)
{
    stored_order order1{trader1, "ETH", sell, 1, 1, 0};
    stored_order order2{trader1, "ETH", sell, 1, 1, 0};
    stored_order order3{trader2, "ETH", buy, 1, 2, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "DEF", "ABC", buy, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETH", "DEF", "ABC", buy, 1, 1);

    ASSERT_EQ(trader2.get_capital_delta(), -2 * 1 * 1.5);
    ASSERT_EQ(trader1.get_capital_delta(), 2 * .5);
}

TEST_F(UnitOrderFeeMatching, MultiplePartialFillReversed)
{
    stored_order order1{trader1, "ETH", sell, 1, 1, 0};
    stored_order order2{trader1, "ETH", sell, 1, 1, 0};
    stored_order order3{trader2, "ETH", buy, 1, 3, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETH", "DEF", "ABC", buy, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETH", "DEF", "ABC", buy, 1, 1);

    ASSERT_EQ(trader2.get_capital_delta(), -2 * 1 * 1.5);
    ASSERT_EQ(trader1.get_capital_delta(), 2 * .5);
}

TEST_F(UnitOrderFeeMatching, NotEnoughToEnough)
{
    trader1.modify_capital(-TEST_STARTING_CAPITAL + 1);

    stored_order order2{trader2, "ETH", sell, 1, 1, 0};
    stored_order order1{trader1, "ETH", buy, 1, 1, 0};

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
    ASSERT_EQ_MATCH(matches[0], "ETH", "ABC", "DEF", buy, 1, 1);

    ASSERT_EQ(trader1.get_capital(), 0);
    ASSERT_EQ(trader2.get_capital_delta(), 1 * .5);
}

TEST_F(UnitOrderFeeMatching, MatchingInvalidFunds)
{
    trader1.modify_capital(-TEST_STARTING_CAPITAL + 1);

    stored_order order1{trader1, "ETH", buy, 1, 1, 0};
    stored_order order2{trader2, "ETH", sell, 1, 1, 0};

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
    nutc::traders::GenericTrader& trader4 =
        *manager_.add_trader<TestTrader>(std::string("A"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader5 =
        *manager_.add_trader<TestTrader>(std::string("B"), 1);
    nutc::traders::GenericTrader& trader6 =
        *manager_.add_trader<TestTrader>(std::string("C"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader7 =
        *manager_.add_trader<TestTrader>(std::string("D"), TEST_STARTING_CAPITAL);

    trader4.modify_holdings("ETH", DEFAULT_QUANTITY);
    trader5.modify_holdings("ETH", DEFAULT_QUANTITY);
    trader6.modify_holdings("ETH", DEFAULT_QUANTITY);
    trader7.modify_holdings("ETH", DEFAULT_QUANTITY);

    stored_order order1{trader4, "ETH", buy, 1, 1, 0};
    stored_order order2{trader5, "ETH", buy, 1, 1, 0};
    stored_order order3{trader6, "ETH", buy, 1, 1, 0};
    stored_order order4{trader7, "ETH", sell, 1, 3, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 0);

    // Should match two orders and throw out the invalid order (2)
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 2);

    ASSERT_EQ_MATCH(matches[0], "ETH", "A", "D", sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETH", "C", "D", sell, 1, 1);

    ASSERT_EQ(trader4.get_capital_delta(), -1.5);
    ASSERT_EQ(trader5.get_capital_delta(), 0);
    ASSERT_EQ(trader6.get_capital_delta(), -1 * 1 * 1.5);
    ASSERT_EQ(trader7.get_capital_delta(), 2 * .5);
}
