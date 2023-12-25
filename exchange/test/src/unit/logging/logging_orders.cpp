#include "matching/engine/engine.hpp"
#include "test_utils/macros.hpp"
#include "utils/messages.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitLoggingOrders : public ::testing::Test {
protected:
    void
    SetUp() override
    {
        manager.add_client("ABC", "ABC");
        manager.add_client("DEF", "DEF");
        manager.modify_holdings("ABC", "ETHUSD", 1000);
        manager.modify_holdings("DEF", "ETHUSD", 1000);
    }

    ClientManager manager;
    Engine engine;
};

TEST_F(UnitLoggingOrders, LogMarketOrders)
{
    manager.modify_capital("ABC", -100000);

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

    auto [matches, ob_updates] = engine.match_order(order1, manager);
    auto [matches2, ob_updates2] = engine.match_order(order2, manager);

    auto& logger = Logger::get_logger();
    EXPECT_NO_FATAL_FAILURE(logger.log_event(matches2.at(0)));
}
