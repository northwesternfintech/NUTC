#include "config.h"
#include "matching/engine/engine.hpp"
#include "test_utils/macros.hpp"
#include "utils/messages.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitLoggingOrders : public ::testing::Test {
protected:
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        using nutc::testing_utils::add_client_simple;

        add_client_simple(manager_, "ABC");
        add_client_simple(manager_, "DEF");

        manager_.modify_holdings("ABC", "ETHUSD", DEFAULT_QUANTITY);
        manager_.modify_holdings("DEF", "ETHUSD", DEFAULT_QUANTITY);
    }

    ClientManager manager_; // NOLINT(*)
    Engine engine_;         // NOLINT(*)
};

TEST_F(UnitLoggingOrders, LogMarketOrders)
{
    manager_.modify_capital("ABC", -STARTING_CAPITAL);

    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};

    auto& logger = Logger::get_logger();

    EXPECT_NO_FATAL_FAILURE(logger.log_event(order1));
    EXPECT_NO_FATAL_FAILURE(logger.log_event(order2));
}

TEST_F(UnitLoggingOrders, LogMatches)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};

    auto [matches, ob_updates] = engine_.match_order(order1, manager_);
    auto [matches2, ob_updates2] = engine_.match_order(order2, manager_);

    auto& logger = Logger::get_logger();
    EXPECT_NO_FATAL_FAILURE(logger.log_event(matches2.at(0)));
}
