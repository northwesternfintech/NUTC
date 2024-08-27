#include "config.h"
#include "exchange/orders/orderbook/cancellable_orderbook.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "shared/types/decimal.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

using nutc::exchange::CancellableOrderBook;
using nutc::exchange::LimitOrderBook;
using nutc::shared::Ticker;
using nutc::shared::Side::buy;
using nutc::shared::Side::sell;

class UnitMatchIOC : public ::testing::Test {
protected:
    using TestTrader = nutc::test::TestTrader;
    static constexpr nutc::shared::decimal_quantity DEFAULT_QUANTITY = 1000.0;

    TraderContainer traders;

    nutc::exchange::GenericTrader& trader1 =
        *traders.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
    nutc::exchange::GenericTrader& trader2 =
        *traders.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);

    void
    SetUp() override
    {
        trader1.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader2.modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    }

    CancellableOrderBook<LimitOrderBook> orderbook_{};
    Engine engine_;

    std::vector<nutc::shared::match>
    add_to_engine_(const tagged_limit_order& order)
    {
        return engine_.match_order(order, orderbook_);
    }
};

TEST_F(UnitMatchIOC, BasicMatchIOC)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0, false};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1, 1.0, true};

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
}

TEST_F(UnitMatchIOC, DoubleIOCMatchMultipleLevels)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 5, 2.0, true};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1, 1.0, false};
    tagged_limit_order order3{trader2, Ticker::ETH, sell, 4, 1.0, false};

    auto matches = add_to_engine_(order2);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order3);
    ASSERT_TRUE(matches.empty());

    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 2);
}

TEST_F(UnitMatchIOC, NoMatchAfterCycle)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0, false};
    tagged_limit_order order2{trader2, Ticker::ETH, sell, 1, 1.0, true};

    add_to_engine_(order2);

    auto matches = add_to_engine_(order1);
    ASSERT_TRUE(matches.empty());
}
