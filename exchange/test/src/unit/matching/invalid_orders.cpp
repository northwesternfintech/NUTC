#include "config.h"
#include "exchange/traders/trader_container.hpp"
#include "shared/util.hpp"
#include "test_utils/helpers/test_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitInvalidOrders : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        manager_.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
        manager_.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);

        manager_.get_trader("ABC")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("DEF")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    }

    TraderContainer& manager_ =
        nutc::traders::TraderContainer::get_instance(); // NOLINT(*)
    Engine engine_{TEST_ORDER_EXPIRATION_TICKS};        // NOLINT (*)

    std::vector<nutc::matching::stored_match>
    add_to_engine_(const stored_order& order)
    {
        return engine_.match_order(order);
    }
};

TEST_F(UnitInvalidOrders, RemoveThenAddFunds)
{
    manager_.get_trader("ABC")->modify_capital(-TEST_STARTING_CAPITAL);

    stored_order order2{
        manager_.get_trader("DEF"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };

    // Thrown out
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    // Kept, but not matched
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    manager_.get_trader("ABC")->modify_capital(TEST_STARTING_CAPITAL);

    // Kept, but not matched
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    // Kept and matched
    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "ABC", "DEF", nutc::util::Side::buy, 1, 1);
}

TEST_F(UnitInvalidOrders, MatchingInvalidFunds)
{
    manager_.get_trader("ABC")->modify_capital(-TEST_STARTING_CAPITAL);

    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("DEF"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };

    // Thrown out
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    // Kept, but not matched
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
}

TEST_F(UnitInvalidOrders, SimpleManyInvalidOrder)
{
    manager_.add_trader<TestTrader>(std::string("A"), TEST_STARTING_CAPITAL);
    manager_.add_trader<TestTrader>(std::string("B"), 0);
    manager_.add_trader<TestTrader>(std::string("C"), TEST_STARTING_CAPITAL);
    manager_.add_trader<TestTrader>(std::string("D"), TEST_STARTING_CAPITAL);

    manager_.get_trader("A")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    manager_.get_trader("B")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    manager_.get_trader("C")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    manager_.get_trader("D")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);

    stored_order order1{
        manager_.get_trader("A"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("B"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order3{
        manager_.get_trader("C"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order4{
        manager_.get_trader("D"), nutc::util::Side::sell, "ETHUSD", 3, 1, 0
    };

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 0);

    // Should match two orders and throw out the invalid order (2)
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 2);

    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "A", "D", nutc::util::Side::sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "C", "D", nutc::util::Side::sell, 1, 1);
}
