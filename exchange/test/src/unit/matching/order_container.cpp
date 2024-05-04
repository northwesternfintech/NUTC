#include "exchange/tickers/engine/order_container.hpp"

#include "config.h"
#include "exchange/traders/trader_container.hpp"
#include "exchange/traders/trader_types/trader_interface.hpp"
#include "test_utils/helpers/test_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

// TODO: expiration tests

using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitOrderContainerTest : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;
    std::shared_ptr<TestTrader> trader_1;
    std::shared_ptr<TestTrader> trader_2;

    void
    SetUp() override
    {
        trader_1 =
            manager_.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
        trader_2 =
            manager_.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);

        trader_1->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        trader_2->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    }

    TraderContainer& manager_ =
        nutc::traders::TraderContainer::get_instance(); // NOLINT(*)
    nutc::matching::OrderContainer container_;          // NOLINT
};

TEST_F(UnitOrderContainerTest, SimpleAddRemove)
{
    stored_order order1{trader_1, nutc::util::Side::buy, "ETHUSD", 1, 1, 1};
    stored_order order2{trader_2, nutc::util::Side::sell, "ETHUSD", 1, 1, 1};

    container_.add_order(order1);
    ASSERT_EQ(container_.get_level(nutc::util::Side::buy, 1), 1);
    ASSERT_FALSE(container_.can_match_orders());

    container_.add_order(order2);
    ASSERT_EQ(container_.get_level(nutc::util::Side::sell, 1), 1);
    ASSERT_TRUE(container_.can_match_orders());

    auto order = container_.remove_order(order1.order_index);
    ASSERT_EQ(order.order_index, order1.order_index);
    ASSERT_EQ(order.price, order1.price);
    ASSERT_EQ(container_.get_level(nutc::util::Side::buy, 1), 0);
    ASSERT_FALSE(container_.can_match_orders());
}

TEST_F(UnitOrderContainerTest, ModifyQuantity)
{
    stored_order so1{trader_1, nutc::util::Side::buy, "ETHUSD", 1, 1, 1};
    stored_order so2{trader_1, nutc::util::Side::sell, "ETHUSD", 1, 1, 1};

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
