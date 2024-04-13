#include "config.h"
#include "exchange/traders/trader_manager.hpp"
#include "exchange/traders/trader_types/local_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitInvalidOrders : public ::testing::Test {
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

    TraderManager& manager_ = nutc::manager::TraderManager::get_instance(); // NOLINT(*)
    Engine engine_{TEST_ORDER_EXPIRATION_TICKS}; // NOLINT (*)

    std::vector<nutc::matching::StoredMatch>
    add_to_engine_(const StoredOrder& order)
    {
        return engine_.match_order(order);
    }
};

TEST_F(UnitInvalidOrders, RemoveThenAddFunds)
{
    manager_.get_trader("ABC")->modify_capital(-TEST_STARTING_CAPITAL);

    StoredOrder order2{manager_.get_trader("DEF"), SELL, "ETHUSD", 1, 1, 0};
    StoredOrder order1{manager_.get_trader("ABC"), BUY, "ETHUSD", 1, 1, 0};

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
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "ABC", "DEF", BUY, 1, 1);
}

TEST_F(UnitInvalidOrders, MatchingInvalidFunds)
{
    manager_.get_trader("ABC")->modify_capital(-TEST_STARTING_CAPITAL);

    StoredOrder order1{manager_.get_trader("ABC"), BUY, "ETHUSD", 1, 1, 0};
    StoredOrder order2{manager_.get_trader("DEF"), SELL, "ETHUSD", 1, 1, 0};

    // Thrown out
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    // Kept, but not matched
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
}

TEST_F(UnitInvalidOrders, SimpleManyInvalidOrder)
{
    manager_.add_trader<LocalTrader>("A", TEST_STARTING_CAPITAL);
    manager_.add_trader<LocalTrader>("B", 0);
    manager_.add_trader<LocalTrader>("C", TEST_STARTING_CAPITAL);
    manager_.add_trader<LocalTrader>("D", TEST_STARTING_CAPITAL);

    manager_.get_trader("A")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    manager_.get_trader("B")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    manager_.get_trader("C")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    manager_.get_trader("D")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);

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

    // Should match two orders and throw out the invalid order (2)
    matches = add_to_engine_(order4);
    ASSERT_EQ(matches.size(), 2);

    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "A", "D", SELL, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "C", "D", SELL, 1, 1);
}
