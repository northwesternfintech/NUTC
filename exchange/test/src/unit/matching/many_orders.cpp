#include "config.h"
#include "exchange/traders/trader_container.hpp"
#include "exchange/traders/trader_types/local_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitManyOrders : public ::testing::Test {
protected:
    using LocalTrader = nutc::traders::LocalTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        manager_.add_trader<LocalTrader>("A", TEST_STARTING_CAPITAL);
        manager_.add_trader<LocalTrader>("B", TEST_STARTING_CAPITAL);
        manager_.add_trader<LocalTrader>("C", TEST_STARTING_CAPITAL);
        manager_.add_trader<LocalTrader>("D", TEST_STARTING_CAPITAL);

        manager_.get_trader("A")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("B")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("C")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("D")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
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

TEST_F(UnitManyOrders, CorrectTimePriority)
{
    stored_order order1{
        manager_.get_trader("A"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("B"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order3{
        manager_.get_trader("C"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    stored_order order4{
        manager_.get_trader("D"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order5{
        manager_.get_trader("C"), nutc::util::Side::sell, "ETHUSD", 5, 1, 0
    };

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "A", "C", nutc::util::Side::sell, 1, 1);
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order5);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "B", "C", nutc::util::Side::sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "D", "C", nutc::util::Side::sell, 1, 1);
}

TEST_F(UnitManyOrders, OnlyMatchesOne)
{
    stored_order order1{
        manager_.get_trader("A"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("B"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "A", "B", nutc::util::Side::sell, 1, 1);
}

TEST_F(UnitManyOrders, SimpleManyOrder)
{
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

    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 3);

    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "A", "D", nutc::util::Side::sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "B", "D", nutc::util::Side::sell, 1, 1);
    ASSERT_EQ_MATCH(matches[2], "ETHUSD", "C", "D", nutc::util::Side::sell, 1, 1);
}

TEST_F(UnitManyOrders, PassiveAndAggressivePartial)
{
    stored_order order1{
        manager_.get_trader("A"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("B"), nutc::util::Side::sell, "ETHUSD", 10, 1, 0
    };
    stored_order order3{
        manager_.get_trader("C"), nutc::util::Side::buy, "ETHUSD", 2, 3, 0
    };
    stored_order order4{
        manager_.get_trader("D"), nutc::util::Side::buy, "ETHUSD", 10, 4, 0
    };

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "C", "A", nutc::util::Side::buy, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "C", "B", nutc::util::Side::buy, 1, 1);
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "D", "B", nutc::util::Side::buy, 1, 9);
}
