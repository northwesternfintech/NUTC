#include "config.h"
#include "exchange/orders/orderbook/cancellable_orderbook.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/traders/trader_container.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

using nutc::matching::CancellableOrderBook;
using nutc::matching::LimitOrderBook;
using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitMatchIOC : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;

    TraderContainer& manager_ = nutc::traders::TraderContainer::get_instance();

    nutc::traders::GenericTrader& trader1 =
        *manager_.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader2 =
        *manager_.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);

    void
    SetUp() override
    {
        trader1.modify_holdings("ETH", DEFAULT_QUANTITY);
        trader2.modify_holdings("ETH", DEFAULT_QUANTITY);
    }

    CancellableOrderBook<LimitOrderBook> orderbook_{};
    Engine engine_;

    std::vector<nutc::matching::stored_match>
    add_to_engine_(const stored_order& order)
    {
		orderbook_.add_order(order);
        return engine_.match_orders(orderbook_);
    }
};

TEST_F(UnitMatchIOC, BasicMatchIOC)
{
    stored_order order1{trader1, "ETH", buy, 1, 1, false};
    stored_order order2{trader2, "ETH", sell, 1, 1, true};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
}

TEST_F(UnitMatchIOC, DoubleIOCMatch)
{
    stored_order order1{trader1, "ETH", buy, 1, 5, true};
    stored_order order2{trader2, "ETH", sell, 1, 0, true};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
}

TEST_F(UnitMatchIOC, DoubleIOCMatchMultipleLevels)
{
    stored_order order1{trader1, "ETH", buy, 2, 5, true};
    stored_order order2{trader2, "ETH", sell, 1, 0, true};
    stored_order order3{trader2, "ETH", sell, 1, 4, true};

    auto matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order3);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 2);
}

TEST_F(UnitMatchIOC, NoMatchAfterCycle)
{
    stored_order order1{trader1, "ETH", buy, 1, 1, false};
    stored_order order2{trader2, "ETH", sell, 1, 1, true};

    add_to_engine_(order2);

    auto removed_orders = orderbook_.remove_ioc_orders();
    ASSERT_EQ(removed_orders.size(), 1);

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());
}
