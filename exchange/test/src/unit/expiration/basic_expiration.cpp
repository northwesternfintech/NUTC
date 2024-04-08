#include "config.h"
#include "exchange/tickers/engine/engine.hpp"
#include "exchange/tickers/engine/order_storage.hpp"
#include "exchange/traders/trader_types/local_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitOrderExpiration : public ::testing::Test {
    using LocalTrader = nutc::manager::LocalTrader;

protected:
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
    add_to_engine_(const MarketOrder& order)
    {
        return engine_.match_order(order);
    }
};

TEST_F(UnitOrderExpiration, SimpleNoMatch)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    ASSERT_EQ(1, engine_.expire_old_orders(TEST_ORDER_EXPIRATION_TICKS).size());
    ASSERT_EQ(0, engine_.get_order_container().get_spread_nums().first);
    ASSERT_EQ(0, engine_.get_order_container().get_spread_nums().second);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
}

TEST_F(UnitOrderExpiration, IncrementTick)
{
    engine_.expire_old_orders(TEST_ORDER_EXPIRATION_TICKS);
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(1, engine_.expire_old_orders(TEST_ORDER_EXPIRATION_TICKS * 2).size());
}
