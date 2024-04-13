#include "config.h"
#include "exchange/traders/trader_manager.hpp"
#include "exchange/traders/trader_types/local_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitManyOrders : public ::testing::Test {
protected:
    using LocalTrader = nutc::manager::LocalTrader;
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

    TraderManager& manager_ = nutc::manager::TraderManager::get_instance(); // NOLINT(*)
    Engine engine_{TEST_ORDER_EXPIRATION_TICKS}; // NOLINT (*)

    std::vector<nutc::matching::StoredMatch>
    add_to_engine_(const StoredOrder& order)
    {
        return engine_.match_order(order);
    }
};

TEST_F(UnitManyOrders, CorrectTimePriority)
{
    StoredOrder order1{manager_.get_trader("A"), BUY, "ETHUSD", 1, 1, 0};
    StoredOrder order2{manager_.get_trader("B"), BUY, "ETHUSD", 1, 1, 0};
    StoredOrder order3{manager_.get_trader("C"), SELL, "ETHUSD", 1, 1, 0};
    StoredOrder order4{manager_.get_trader("D"), BUY, "ETHUSD", 1, 1, 0};
    StoredOrder order5{manager_.get_trader("C"), SELL, "ETHUSD", 5, 1, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "A", "C", SELL, 1, 1);
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order5);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "B", "C", SELL, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "D", "C", SELL, 1, 1);
}

TEST_F(UnitManyOrders, OnlyMatchesOne)
{
    StoredOrder order1{manager_.get_trader("A"), BUY, "ETHUSD", 1, 1, 0};
    StoredOrder order2{manager_.get_trader("B"), SELL, "ETHUSD", 1, 1, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "A", "B", SELL, 1, 1);
}

TEST_F(UnitManyOrders, SimpleManyOrder)
{
    StoredOrder order1{manager_.get_trader("A"), BUY, "ETHUSD", 1, 1, 0};
    StoredOrder order2{manager_.get_trader("B"), BUY, "ETHUSD", 1, 1, 0};
    StoredOrder order3{manager_.get_trader("C"), BUY, "ETHUSD", 1, 1, 0};
    StoredOrder order4{manager_.get_trader("D"), SELL, "ETHUSD", 3, 1, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 3);

    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "A", "D", SELL, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "B", "D", SELL, 1, 1);
    ASSERT_EQ_MATCH(matches[2], "ETHUSD", "C", "D", SELL, 1, 1);
}

TEST_F(UnitManyOrders, PassiveAndAggressivePartial)
{
    StoredOrder order1{manager_.get_trader("A"), SELL, "ETHUSD", 1, 1, 0};
    StoredOrder order2{manager_.get_trader("B"), SELL, "ETHUSD", 10, 1, 0};
    StoredOrder order3{manager_.get_trader("C"), BUY, "ETHUSD", 2, 3, 0};
    StoredOrder order4{manager_.get_trader("D"), BUY, "ETHUSD", 10, 4, 0};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "C", "A", BUY, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "C", "B", BUY, 1, 1);
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "D", "B", BUY, 1, 9);
}
