#include "exchange/config.h"
#include "exchange/tickers/engine/engine.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitLoggingOrders : public ::testing::Test {
protected:
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        manager_.add_local_trader("ABC", STARTING_CAPITAL);
        manager_.add_local_trader("DEF", STARTING_CAPITAL);

        manager_.get_trader("ABC")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("DEF")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    }

    ClientManager& manager_ = nutc::manager::ClientManager::get_instance(); // NOLINT(*)
    Engine engine_;                                                         // NOLINT(*)

    nutc::matching::match_result_t
    add_to_engine_(MarketOrder order)
    {
        return engine_.match_order(std::move(order), manager_);
    }
};

TEST_F(UnitLoggingOrders, LogMarketOrders)
{
    manager_.get_trader("ABC")->modify_capital(-STARTING_CAPITAL);

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

    auto [matches, ob_updates] = add_to_engine_(order1);
    auto [matches2, ob_updates2] = add_to_engine_(order2);

    auto& logger = Logger::get_logger();
    // EXPECT_NO_FATAL_FAILURE(logger.log_event(matches2.at(0)));
}
