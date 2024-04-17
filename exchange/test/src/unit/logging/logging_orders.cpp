#include "config.h"
#include "exchange/tickers/engine/order_storage.hpp"
#include "exchange/traders/trader_manager.hpp"
#include "exchange/traders/trader_types/local_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitLoggingOrders : public ::testing::Test {
protected:
    static constexpr const int DEFAULT_QUANTITY = 1000;
    using LocalTrader = nutc::manager::LocalTrader;

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

TEST_F(UnitLoggingOrders, Logmarket_orders)
{
    manager_.get_trader("ABC")->modify_capital(-TEST_STARTING_CAPITAL);

    market_order order2{"DEF", nutc::util::Side::sell, "ETHUSD", 1, 1};
    market_order order1{"ABC", nutc::util::Side::buy, "ETHUSD", 1, 1};

    auto& logger = Logger::get_logger();

    EXPECT_NO_FATAL_FAILURE(logger.log_event(order1));
    EXPECT_NO_FATAL_FAILURE(logger.log_event(order2));
}

TEST_F(UnitLoggingOrders, LogMatches)
{
    StoredOrder order1{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    StoredOrder order2{
        manager_.get_trader("DEF"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };

    auto matches = add_to_engine_(order1);
    auto matches2 = add_to_engine_(order2);

    auto& logger = Logger::get_logger();
    // EXPECT_NO_FATAL_FAILURE(logger.log_event(matches2.at(0)));
}
