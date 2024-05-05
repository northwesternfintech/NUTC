#include "config.h"
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
    Engine engine_{TEST_ORDER_EXPIRATION_TICKS};        // NOLINT (*)

    std::vector<nutc::matching::stored_match>
    add_to_engine_(const stored_order& order)
    {
        return engine_.match_order(order);
    }
};

TEST_F(UnitOrderExpiration, SimpleNoMatch)
{
    stored_order order1{trader1, buy, "ETHUSD", 1, 1, 0};
    stored_order order2{trader2, sell, "ETHUSD", 1, 1, 0};
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
    stored_order order1{trader1, buy, "ETHUSD", 1, 1, TEST_ORDER_EXPIRATION_TICKS};
    stored_order order2{trader2, sell, "ETHUSD", 1, 1, TEST_ORDER_EXPIRATION_TICKS};

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(1, engine_.expire_old_orders(TEST_ORDER_EXPIRATION_TICKS * 2).size());
}
