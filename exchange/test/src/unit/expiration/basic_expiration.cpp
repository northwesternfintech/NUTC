#include "exchange/config.h"
#include "exchange/tickers/engine/engine.hpp"
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

    nutc::matching::match_result_t
    add_to_engine_(MarketOrder order)
    {
        return engine_.match_order(std::move(order), manager_);
    }
};

TEST_F(UnitOrderExpiration, SimpleNoMatch)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", BUY, 1, 1);

    ASSERT_EQ(1, engine_.on_tick(1, 1).removed_orders.size());

    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", SELL, 1, 1);
}

TEST_F(UnitOrderExpiration, IncrementTick)
{
    engine_.on_tick(1, 10);

    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};

    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(1, engine_.on_tick(2, 1).removed_orders.size());
}
