#include "exchange/config.h"
#include "exchange/tickers/engine/new_engine.hpp"
#include "exchange/tickers/engine/order_storage.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitOrderExpiration : public ::testing::Test {
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
    Engine engine_{}; // NOLINT (*)

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

    ASSERT_EQ(1, engine_.expire_old_orders(ORDER_EXPIRATION_TIME).size());
    ASSERT_EQ(0, engine_.get_order_container().get_spread_nums().first);
    ASSERT_EQ(0, engine_.get_order_container().get_spread_nums().second);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
}

TEST_F(UnitOrderExpiration, IncrementTick)
{
    engine_.expire_old_orders(ORDER_EXPIRATION_TIME);
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(1, engine_.expire_old_orders(ORDER_EXPIRATION_TIME*2).size());
}
