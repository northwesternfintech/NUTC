#include "exchange/tickers/engine/order_container.hpp"

#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitOrderContainerTest : public ::testing::Test {
protected:
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        manager_.add_local_trader("ABC");
        manager_.add_local_trader("DEF");

        manager_.get_trader("ABC")->modify_capital(STARTING_CAPITAL);
        manager_.get_trader("DEF")->modify_capital(STARTING_CAPITAL);

        manager_.get_trader("ABC")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("DEF")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    }

    ClientManager& manager_ = nutc::manager::ClientManager::get_instance(); // NOLINT(*)
    nutc::matching::OrderContainer container_;                              // NOLINT
};

TEST_F(UnitOrderContainerTest, SimpleAddRemove)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", SELL, "ETHUSD", 1, 1};
    StoredOrder so1 = StoredOrder(std::move(order1), 0);
    StoredOrder so2 = StoredOrder(std::move(order2), 0);

    container_.add_order(so1);
    ASSERT_EQ(container_.get_level(SIDE::BUY, 1), 1);
    ASSERT_FALSE(container_.can_match_orders());
  
    container_.add_order(so2);
    ASSERT_EQ(container_.get_level(SIDE::SELL, 1), 1);
    ASSERT_TRUE(container_.can_match_orders());

    container_.remove_order(so1.order_index);
    ASSERT_EQ(container_.get_level(SIDE::BUY, 1), 0);
    ASSERT_FALSE(container_.can_match_orders());
}

TEST_F(UnitOrderContainerTest, ModifyQuantity)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", SELL, "ETHUSD", 1, 1};
    StoredOrder so1 = StoredOrder(std::move(order1), 0);
    StoredOrder so2 = StoredOrder(std::move(order2), 0);

    container_.add_order(so1);
    container_.add_order(so2);
    ASSERT_EQ(container_.get_level(SIDE::SELL, 1), 1);
    ASSERT_TRUE(container_.can_match_orders());

    container_.modify_order_quantity(so2.order_index, 1);
    ASSERT_EQ(container_.get_level(SIDE::SELL, 1), 2);
  
    container_.modify_order_quantity(so2.order_index, -2);
    ASSERT_EQ(container_.get_level(SIDE::SELL, 1), 0);
    ASSERT_FALSE(container_.can_match_orders());
}
