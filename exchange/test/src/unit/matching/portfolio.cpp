#include "common/types/decimal.hpp"
#include "config.h"
#include "exchange/matching/engine.hpp"
#include "exchange/orders/orderbook/composite_orderbook.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

using nutc::common::Ticker;
using nutc::common::Side::buy;
using nutc::common::Side::sell;

class UnitPortfolio : public ::testing::Test {
protected:
    using TestTrader = nutc::test::TestTrader;
    static constexpr nutc::common::decimal_quantity DEFAULT_QUANTITY = 1000.0;

    TestTrader trader1{"ABC", TEST_STARTING_CAPITAL};
    TestTrader trader2{"DEF", TEST_STARTING_CAPITAL};

    nutc::exchange::CompositeOrderBook orderbook_{Ticker::ETH};

    std::vector<nutc::common::match>
    add_to_engine_(const tagged_limit_order& order)
    {
        return nutc::exchange::match_order(order, orderbook_);
    }

    void
    SetUp() override
    {
        trader1.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
        trader2.get_portfolio().modify_holdings(Ticker::ETH, DEFAULT_QUANTITY);
    }
};

TEST_F(UnitPortfolio, CompletedOrdersClearInterest)
{
    tagged_limit_order buy_order{trader1, Ticker::ETH, buy, 123.0, 312.0};
    auto matches = add_to_engine_(buy_order);
    ASSERT_TRUE(matches.empty());

    tagged_limit_order sell_order1{trader2, Ticker::ETH, sell, 123.0/2.0, 312.0};
    matches = add_to_engine_(sell_order1);
    ASSERT_EQ(matches.size(), 1);

    tagged_limit_order sell_order2{trader2, Ticker::ETH, sell, 123.0/2.0, 312.0};
    matches = add_to_engine_(sell_order2);
    ASSERT_EQ(matches.size(), 1);

	EXPECT_EQ(trader1.get_portfolio().get_capital_utilization(), 0.0);
	EXPECT_EQ(trader2.get_portfolio().get_capital_utilization(), 0.0);
}
