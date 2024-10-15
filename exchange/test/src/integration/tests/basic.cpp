#include "common/util.hpp"
#include "config.h"
#include "util/helpers/test_cycle.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"
#include "util/process.hpp"

#include <gtest/gtest.h>

namespace nutc::test {
using common::limit_order;
using nutc::common::AlgoLanguage;
using nutc::common::Ticker;
using nutc::common::Side::buy;
using nutc::common::Side::sell;
using nutc::test::start_wrappers;

class IntegrationBasicAlgo : public ::testing::TestWithParam<AlgoLanguage> {
protected:
    TraderContainer traders_;
};

TEST_P(IntegrationBasicAlgo, ConfirmOrderReceived)
{
    start_wrappers(traders_, GetParam(), "buy_tsla_at_100");
    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader2->get_portfolio().modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order(make_limit_order(Ticker::ETH, sell, 100.0, 10.0));

    TestMatchingCycle cycle{traders_};

    cycle.wait_for_order(make_limit_order(Ticker::ETH, buy, 100.0, 10.0));
    ASSERT_EQ(
        double{
            trader2->get_portfolio().get_capital()
            - trader2->get_portfolio().get_initial_capital()
        },
        100.0 * 10.0
    );
}

TEST_P(IntegrationBasicAlgo, ConfirmOrderFeeApplied)
{
    start_wrappers(traders_, GetParam(), "buy_tsla_at_100");
    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader2->get_portfolio().modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order(make_limit_order(Ticker::ETH, sell, 100.0, 10.0));

    TestMatchingCycle cycle{traders_, .5};

    cycle.wait_for_order(make_limit_order(Ticker::ETH, buy, 100.0, 10.0));
    ASSERT_EQ(
        double{
            trader2->get_portfolio().get_capital()
            - trader2->get_portfolio().get_initial_capital()
        },
        100.0 * 10.0 / 2
    );
}

TEST_P(IntegrationBasicAlgo, RemoveIOCOrder)
{
    auto& trader1 = start_wrappers(traders_, GetParam(), "buy_tsla_at_100");
    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader2->get_portfolio().modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order(make_limit_order(Ticker::ETH, sell, 100.0, 100.0));

    TestMatchingCycle cycle{traders_};

    cycle.on_tick(0);
    usleep(500);
    cycle.on_tick(0);
    usleep(500);
    cycle.on_tick(0);

    auto orders = trader1.read_orders();
    ASSERT_TRUE(orders.empty());
}

TEST_P(IntegrationBasicAlgo, MarketOrderBuy)
{
    start_wrappers(traders_, GetParam(), "buy_market_order_1000");
    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader2->get_portfolio().modify_holdings(Ticker::ETH, 1000.0);
    trader2->add_order(make_limit_order(Ticker::ETH, sell, 100.0, 100.0));

    TestMatchingCycle cycle{traders_};

    cycle.wait_for_order(make_limit_order(Ticker::BTC, buy, 1.0, 100.0));
}

TEST_P(IntegrationBasicAlgo, MarketOrderSell)
{
    auto& trader1 = start_wrappers(traders_, GetParam(), "sell_market_order_1000");
    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader1.get_portfolio().modify_holdings(Ticker::ETH, 1000.0);
    trader2->add_order(make_limit_order(Ticker::ETH, buy, 1.0, 100.0));
    trader2->get_portfolio().modify_capital(1000.0);

    TestMatchingCycle cycle{traders_};

    cycle.wait_for_order(make_limit_order(Ticker::BTC, buy, 1.0, 100.0));
}

TEST_P(IntegrationBasicAlgo, ManyUpdates)
{
    start_wrappers(traders_, GetParam(), "confirm_1000");
    auto trader2 = traders_.add_trader<TestTrader>(0);

    trader2->get_portfolio().modify_holdings(Ticker::ETH, 100000.0); // NOLINT

    TestMatchingCycle cycle{traders_};

    for (int i = 0; i < 10000; i++) {
        trader2->add_order(
            make_limit_order(Ticker::ETH, sell, 1.0, static_cast<double>(i))
        );
    }

    cycle.on_tick(0);

    cycle.wait_for_order(make_limit_order(Ticker::ETH, buy, 10.0, 100.0));
}

TEST_P(IntegrationBasicAlgo, OrderVolumeLimitsPreventGoingAboveLimit)
{
    auto& trader1 = start_wrappers(traders_, GetParam(), "many_orders");

    TestMatchingCycle cycle{traders_, 0.0, 10.0};

    for (int i = 1; i < 21; i++) {
        cycle.wait_for_order(
            make_limit_order(Ticker::ETH, buy, 1.0, static_cast<double>(i))
        );
        cycle.on_tick(0);
    }

    ASSERT_EQ(static_cast<double>(trader1.get_portfolio().get_open_bids()), 10.0);
}

TEST_P(IntegrationBasicAlgo, OnTradeUpdate)
{
    start_wrappers(traders_, GetParam(), "buy_tsla_on_trade");

    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader2->get_portfolio().modify_holdings(Ticker::ETH, 10000.0); // NOLINT

    TestMatchingCycle cycle{traders_};

    trader2->add_order(make_limit_order(Ticker::ETH, sell, 100.0, 100.0));

    cycle.wait_for_order(make_limit_order(Ticker::ETH, buy, 10.0, 102.0));

    cycle.wait_for_order(make_limit_order(Ticker::BTC, buy, 1.0, 100.0));
}

// Sanity check that it goes through the orderbook
TEST_P(IntegrationBasicAlgo, MultipleLevelOrder)
{
    auto& trader1 = start_wrappers(traders_, GetParam(), "buy_tsla_at_100");

    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader2->get_portfolio().modify_holdings(Ticker::ETH, 1000.0); // NOLINT

    TestMatchingCycle cycle{traders_};

    trader2->add_order(make_limit_order(Ticker::ETH, sell, 55.0, 1.0));
    trader2->add_order(make_limit_order(Ticker::ETH, sell, 45.0, 1.0));

    cycle.wait_for_order(make_limit_order(Ticker::ETH, buy, 100.0, 10.0));
    ASSERT_EQ(
        trader1.get_portfolio().get_capital()
            - trader1.get_portfolio().get_initial_capital(),
        -100.0
    );
}

TEST_P(IntegrationBasicAlgo, OnAccountUpdateSell)
{
    auto& trader1 = start_wrappers(traders_, GetParam(), "sell_tsla_on_account");
    trader1.get_portfolio().modify_holdings(Ticker::ETH, 1000.0);

    auto trader2 = traders_.add_trader<TestTrader>(100000);
    trader2->add_order(make_limit_order(Ticker::ETH, buy, 102.0, 102.0));

    TestMatchingCycle cycle{traders_};

    // obupdate triggers one user to place acommon::Side::buy order of 10 ABC at 102
    cycle.wait_for_order(make_limit_order(Ticker::ETH, sell, 10.0, 100.0));

    // on_trade_match triggers one user to place acommon::Side::buy order of 1 ABC
    // at 100
    cycle.wait_for_order(make_limit_order(Ticker::BTC, buy, 1.0, 100.0));
}

TEST_P(IntegrationBasicAlgo, OnAccountUpdateBuy)
{
    start_wrappers(traders_, GetParam(), "buy_tsla_on_account");

    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader2->get_portfolio().modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order(make_limit_order(Ticker::ETH, sell, 100.0, 100.0));

    TestMatchingCycle cycle{traders_};

    // obupdate triggers one user to place acommon::Side::buy order of 10 ABC at 102
    cycle.wait_for_order(make_limit_order(Ticker::ETH, buy, 10.0, 102.0));
    // on_trade_match triggers one user to place acommon::Side::buy order of 1 ABC
    // at 100
    cycle.wait_for_order(make_limit_order(Ticker::BTC, buy, 1.0, 100.0));
}

TEST_P(IntegrationBasicAlgo, AlgoStartDelay)
{
    start_wrappers(
        traders_, GetParam(), "buy_tsla_at_100", TEST_STARTING_CAPITAL,
        TEST_CLIENT_WAIT_SECS
    );

    auto start = std::chrono::high_resolution_clock::now();

    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader2->get_portfolio().modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order(make_limit_order(Ticker::ETH, sell, 100.0, 100.0));

    TestMatchingCycle cycle{traders_};

    cycle.wait_for_order(make_limit_order(Ticker::ETH, buy, 100.0, 10.0));

    auto end = std::chrono::high_resolution_clock::now();
    const int64_t observed_duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double wait_time_ms = TEST_CLIENT_WAIT_SECS * 1000;

    EXPECT_GE(observed_duration_ms, .9 * wait_time_ms);
    EXPECT_LE(observed_duration_ms, 1.5 * wait_time_ms);
}

// Disable trader and confirm it doesn't send any orders
TEST_P(IntegrationBasicAlgo, DisableTrader)
{
    auto& trader1 = start_wrappers(traders_, GetParam(), "buy_tsla_at_100");
    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader2->get_portfolio().modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order(make_limit_order(Ticker::ETH, sell, 100.0, 100.0));

    trader1.disable();

    TestMatchingCycle cycle{traders_};

    cycle.on_tick(0);
    sleep(1);
    auto orders = trader1.read_orders();
    ASSERT_TRUE(orders.empty());
}

INSTANTIATE_TEST_SUITE_P(
    IntegrationBasic, IntegrationBasicAlgo,
    ::testing::Values(AlgoLanguage::python, AlgoLanguage::cpp)
);
} // namespace nutc::test
