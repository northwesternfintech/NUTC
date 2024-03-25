#include "exchange/config.h"
#include "exchange/tickers/engine/engine.hpp"
#include "exchange/traders/trader_manager.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitManyOrders : public ::testing::Test {
protected:
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        manager_.add_local_trader("A", STARTING_CAPITAL);
        manager_.add_local_trader("B", STARTING_CAPITAL);
        manager_.add_local_trader("C", STARTING_CAPITAL);
        manager_.add_local_trader("D", STARTING_CAPITAL);

        manager_.get_trader("A")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("B")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("C")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("D")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    }

    TraderManager& manager_ = nutc::manager::TraderManager::get_instance(); // NOLINT(*)
    Engine engine_{}; // NOLINT (*)

    std::vector<nutc::matching::StoredMatch>
    add_to_engine_(MarketOrder order)
    {
        return engine_.match_order(std::move(order));
    }
};

TEST_F(UnitManyOrders, CorrectTimePriority)
{
    MarketOrder order1{"A", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"B", BUY, "ETHUSD", 1, 1};
    MarketOrder order3{"C", SELL, "ETHUSD", 1, 1};
    MarketOrder order4{"D", BUY, "ETHUSD", 1, 1};
    MarketOrder order5{"C", SELL, "ETHUSD", 5, 1};

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
    MarketOrder order1{"A", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"B", SELL, "ETHUSD", 1, 1};

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
    MarketOrder order1{"A", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"B", BUY, "ETHUSD", 1, 1};
    MarketOrder order3{"C", BUY, "ETHUSD", 1, 1};
    MarketOrder order4{"D", SELL, "ETHUSD", 3, 1};

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
    MarketOrder order1{"A", SELL, "ETHUSD", 1, 1};
    MarketOrder order2{"B", SELL, "ETHUSD", 10, 1}; // NOLINT (*)
    MarketOrder order3{"C", BUY, "ETHUSD", 2, 3};
    MarketOrder order4{"D", BUY, "ETHUSD", 10, 4}; // NOLINT (*)

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
