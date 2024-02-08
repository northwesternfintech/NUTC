#include "exchange/tickers/engine/engine.hpp"
#include "exchange/tickers/engine/order_storage.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
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
        using nutc::testing_utils::add_client_simple;
        using nutc::testing_utils::modify_holdings_simple;

        add_client_simple(manager_, "ABC");
        add_client_simple(manager_, "DEF");

        modify_holdings_simple(manager_, "ABC", "ETHUSD", DEFAULT_QUANTITY);
        modify_holdings_simple(manager_, "DEF", "ETHUSD", DEFAULT_QUANTITY);
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

    ASSERT_EQ(1, engine_.remove_old_orders(1, 0).first.size());

    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", SELL, 1, 1);
}

TEST_F(UnitOrderExpiration, IncrementTick)
{
    engine_.remove_old_orders(1, 0);

    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};

    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(1, engine_.remove_old_orders(2, 1).first.size());
}
