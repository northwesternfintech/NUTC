#include "exchange/tickers/engine/order_container.hpp"

#include "config.h"
#include "exchange/traders/trader_container.hpp"
#include "exchange/traders/trader_types/local_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

// TODO: expiration tests

using nutc::testing_utils::make_stored_order;
using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitOrderContainerTest : public ::testing::Test {
protected:
    using LocalTrader = nutc::traders::LocalTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        manager_.add_trader<LocalTrader>("ABC", TEST_STARTING_CAPITAL);
        manager_.add_trader<LocalTrader>("DEF", TEST_STARTING_CAPITAL);

        manager_.get_trader("ABC")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("DEF")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    }

    TraderContainer& manager_ =
        nutc::traders::TraderContainer::get_instance(); // NOLINT(*)
    nutc::matching::OrderContainer container_;          // NOLINT
};

TEST_F(UnitOrderContainerTest, SimpleAddRemove)
{
    market_order order1{"ABC", nutc::util::Side::buy, "ETHUSD", 1, 1};
    market_order order2{"ABC", nutc::util::Side::sell, "ETHUSD", 1, 1};
    stored_order so1 = make_stored_order(order1, manager_);
    stored_order so2 = make_stored_order(order2, manager_);

    container_.add_order(so1);
    ASSERT_EQ(container_.get_level(nutc::util::Side::buy, 1), 1);
    ASSERT_FALSE(container_.can_match_orders());

    container_.add_order(so2);
    ASSERT_EQ(container_.get_level(nutc::util::Side::sell, 1), 1);
    ASSERT_TRUE(container_.can_match_orders());

    auto order = container_.remove_order(so1.order_index);
    ASSERT_EQ(order.order_index, so1.order_index);
    ASSERT_EQ(order.price, so1.price);
    ASSERT_EQ(container_.get_level(nutc::util::Side::buy, 1), 0);
    ASSERT_FALSE(container_.can_match_orders());
}

TEST_F(UnitOrderContainerTest, ModifyQuantity)
{
    market_order order1{"ABC", nutc::util::Side::buy, "ETHUSD", 1, 1};
    market_order order2{"ABC", nutc::util::Side::sell, "ETHUSD", 1, 1};
    stored_order so1 = make_stored_order(order1, manager_);
    stored_order so2 = make_stored_order(order2, manager_);

    container_.add_order(so1);
    container_.add_order(so2);
    ASSERT_EQ(container_.get_level(nutc::util::Side::sell, 1), 1);
    ASSERT_TRUE(container_.can_match_orders());

    container_.modify_order_quantity(so2.order_index, 1);
    ASSERT_EQ(container_.get_level(nutc::util::Side::sell, 1), 2);

    container_.modify_order_quantity(so2.order_index, -2);
    ASSERT_EQ(container_.get_level(nutc::util::Side::sell, 1), 0);
    ASSERT_FALSE(container_.can_match_orders());
}
