#include "config.h"
#include "exchange/tickers/engine/order_container.hpp"
#include "exchange/tickers/engine/order_storage.hpp"
#include "test_utils/helpers/test_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitOrderExpiration : public ::testing::Test {
    using TestTrader = nutc::test_utils::TestTrader;

protected:
    static constexpr const int DEFAULT_QUANTITY = 1000;
    std::shared_ptr<nutc::traders::GenericTrader> trader1, trader2;

    void
    SetUp() override
    {
        trader1 =
            manager_.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
        trader2 =
            manager_.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);

        trader1->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        trader2->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    }

    TraderContainer& manager_ =
        nutc::traders::TraderContainer::get_instance(); // NOLINT(*)
    nutc::matching::OrderBook orderbook_;
    Engine engine_{TEST_ORDER_EXPIRATION_TICKS}; // NOLINT (*)

    std::vector<nutc::matching::stored_match>
    add_to_engine_(const stored_order& order)
    {
        return engine_.match_order(orderbook_, order);
    }
};

TEST_F(UnitOrderExpiration, SimpleNoMatch)
{
    stored_order order1{trader1, buy, "ETHUSD", 1, 1, 0};
    stored_order order2{trader2, sell, "ETHUSD", 1, 1, 0};
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    auto expired = orderbook_.expire_orders(0);
    ASSERT_EQ(1, expired.size());
    ASSERT_EQ(expired.at(0), order1);

    // TODO: make a macro for if they aren't equal
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    expired = orderbook_.expire_orders(0);
    ASSERT_EQ(1, expired.size());
    ASSERT_EQ(expired.at(0), order2);
}

TEST_F(UnitOrderExpiration, IncrementTick)
{
    orderbook_.expire_orders(TEST_ORDER_EXPIRATION_TICKS);
    stored_order order1{trader1, buy, "ETHUSD", 1, 1, TEST_ORDER_EXPIRATION_TICKS};
    stored_order order2{trader2, sell, "ETHUSD", 1, 1, TEST_ORDER_EXPIRATION_TICKS};

    add_to_engine_(order1);
    auto expired = orderbook_.expire_orders(TEST_ORDER_EXPIRATION_TICKS);
    ASSERT_EQ(1, expired.size());
    ASSERT_EQ(expired.at(0), order1);
}

TEST_F(UnitOrderExpiration, ExpireOne)
{
    orderbook_.expire_orders(TEST_ORDER_EXPIRATION_TICKS);
    stored_order order1{trader1, buy, "IDK", 1, 1, TEST_ORDER_EXPIRATION_TICKS};
    stored_order order2{trader2, sell, "ETHUSD", 1, 1, TEST_ORDER_EXPIRATION_TICKS + 1};

    add_to_engine_(order1);
    add_to_engine_(order2);
    auto expired = orderbook_.expire_orders(TEST_ORDER_EXPIRATION_TICKS);
    ASSERT_EQ(1, expired.size());
    ASSERT_EQ(expired.at(0), order1);
}

// More of an integration test between engine and orderbook
TEST_F(UnitOrderExpiration, PartialExpiration)
{
    orderbook_.expire_orders(TEST_ORDER_EXPIRATION_TICKS);
    stored_order order1{trader1, buy, "ETHUSD", 1, 1, TEST_ORDER_EXPIRATION_TICKS};
    stored_order order2{trader2, sell, "ETHUSD", 2, 1, TEST_ORDER_EXPIRATION_TICKS};

    add_to_engine_(order1);
    add_to_engine_(order2);

    // Confirms the expired order is the modified order 2 with reduced quantity
    auto expired = orderbook_.expire_orders(TEST_ORDER_EXPIRATION_TICKS);
    ASSERT_EQ(1, expired.size());
    stored_order test_eq{trader2, sell, "ETHUSD", 1, 1, TEST_ORDER_EXPIRATION_TICKS};
    ASSERT_EQ(expired.at(0), test_eq);
}
