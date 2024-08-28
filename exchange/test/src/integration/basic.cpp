#include "config.h"
#include "util/helpers/test_cycle.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"
#include "util/process.hpp"

#include <gtest/gtest.h>

namespace nutc::test {
using common::limit_order;
using nutc::common::Ticker;
using nutc::common::Side::buy;
using nutc::common::Side::sell;
using nutc::test::start_wrappers;

class IntegrationBasicAlgo : public ::testing::Test {
protected:
    TraderContainer traders;
};

TEST_F(IntegrationBasicAlgo, InitialLiquidity)
{
    start_wrappers(traders, "test_algos/buy_tsla_at_100.py");
    auto trader2 = traders.add_trader<TestTrader>(0);
    trader2->modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order(limit_order{Ticker::ETH, sell, 100.0, 100.0});

    TestMatchingCycle cycle{traders};

    cycle.wait_for_order(limit_order{Ticker::ETH, buy, 100.0, 10.0});
}

TEST_F(IntegrationBasicAlgo, RemoveIOCOrder)
{
    auto& trader1 = start_wrappers(traders, "test_algos/buy_tsla_at_100.py");
    auto trader2 = traders.add_trader<TestTrader>(0);
    trader2->modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order({Ticker::ETH, sell, 100.0, 100.0});

    TestMatchingCycle cycle{traders};

    cycle.on_tick(0);
    usleep(500);
    cycle.on_tick(0);
    usleep(500);
    cycle.on_tick(0);

    auto orders = trader1.read_orders();
    ASSERT_TRUE(orders.empty());
}

TEST_F(IntegrationBasicAlgo, MarketOrderBuy)
{
    start_wrappers(traders, "test_algos/buy_market_order_1000.py");
    auto trader2 = traders.add_trader<TestTrader>(0);
    trader2->modify_holdings(Ticker::ETH, 1000.0);
    trader2->add_order({Ticker::ETH, sell, 100.0, 100.0});

    TestMatchingCycle cycle{traders};

    cycle.wait_for_order(limit_order{Ticker::BTC, buy, 1, 100.0});
}

TEST_F(IntegrationBasicAlgo, MarketOrderSell)
{
    auto& trader1 = start_wrappers(traders, "test_algos/sell_market_order_1.py");
    auto trader2 = traders.add_trader<TestTrader>(0);
    trader1.modify_holdings(Ticker::ETH, 1000.0);
    trader2->add_order({Ticker::ETH, buy, 1, 100.0});
    trader2->modify_capital(1000.0);

    TestMatchingCycle cycle{traders};

    cycle.wait_for_order(limit_order{Ticker::BTC, buy, 1, 100.0});
}

TEST_F(IntegrationBasicAlgo, ManyUpdates)
{
    start_wrappers(traders, "test_algos/confirm_1000.py");
    auto trader2 = traders.add_trader<TestTrader>(0);

    trader2->modify_holdings(Ticker::ETH, 100000.0); // NOLINT

    TestMatchingCycle cycle{traders};

    for (double i = 0; i < 10000; i++) {
        trader2->add_order({Ticker::ETH, sell, 1, i / 100});
    }

    cycle.on_tick(0);

    cycle.wait_for_order(limit_order{Ticker::ETH, buy, 10, 100.0});
}

TEST_F(IntegrationBasicAlgo, OnTradeUpdate)
{
    start_wrappers(traders, "test_algos/buy_tsla_on_trade.py");

    auto trader2 = traders.add_trader<TestTrader>(0);
    trader2->modify_holdings(Ticker::ETH, 10000.0); // NOLINT

    TestMatchingCycle cycle{traders};

    trader2->add_order({Ticker::ETH, sell, 100.0, 100.0});

    cycle.wait_for_order(limit_order{Ticker::ETH, buy, 10, 102.0});

    cycle.wait_for_order(limit_order{Ticker::BTC, buy, 1, 100.0});
}

// Sanity check that it goes through the orderbook
TEST_F(IntegrationBasicAlgo, MultipleLevelOrder)
{
    auto& trader1 = start_wrappers(traders, "test_algos/buy_tsla_at_100.py");

    auto trader2 = traders.add_trader<TestTrader>(0);
    trader2->modify_holdings(Ticker::ETH, 1000.0); // NOLINT

    TestMatchingCycle cycle{traders};

    trader2->add_order({Ticker::ETH, sell, 55, 1.0});
    trader2->add_order({Ticker::ETH, sell, 45, 1.0});

    cycle.wait_for_order(limit_order{Ticker::ETH, buy, 100.0, 10.0});
    ASSERT_EQ(trader1.get_capital() - trader1.get_initial_capital(), -100.0);
}

TEST_F(IntegrationBasicAlgo, OnAccountUpdateSell)
{
    auto& trader1 = start_wrappers(traders, "test_algos/sell_tsla_on_account.py");
    trader1.modify_holdings(Ticker::ETH, 1000.0);

    auto trader2 = traders.add_trader<TestTrader>(100000);
    trader2->add_order({Ticker::ETH, buy, 102, 102.0});

    TestMatchingCycle cycle{traders};

    // obupdate triggers one user to place acommon::Side::buy order of 10 ABC at 102
    cycle.wait_for_order(limit_order{Ticker::ETH, sell, 10, 100.0});

    // on_trade_match triggers one user to place acommon::Side::buy order of 1 ABC at
    // 100
    cycle.wait_for_order(limit_order{Ticker::BTC, buy, 1, 100.0});
}

TEST_F(IntegrationBasicAlgo, OnAccountUpdateBuy)
{
    start_wrappers(traders, "test_algos/buy_tsla_on_account.py");

    auto trader2 = traders.add_trader<TestTrader>(0);
    trader2->modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order({Ticker::ETH, sell, 100.0, 100.0});

    TestMatchingCycle cycle{traders};

    // obupdate triggers one user to place acommon::Side::buy order of 10 ABC at 102
    cycle.wait_for_order(limit_order{Ticker::ETH, buy, 10, 102.0});
    // on_trade_match triggers one user to place acommon::Side::buy order of 1 ABC at
    // 100
    cycle.wait_for_order(limit_order{Ticker::BTC, buy, 1, 100.0});
}

TEST_F(IntegrationBasicAlgo, AlgoStartDelay)
{
    start_wrappers(
        traders, "test_algos/buy_tsla_at_100.py", TEST_STARTING_CAPITAL,
        TEST_CLIENT_WAIT_SECS
    );

    auto start = std::chrono::high_resolution_clock::now();

    auto trader2 = traders.add_trader<TestTrader>(0);
    trader2->modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order({Ticker::ETH, sell, 100.0, 100.0});

    TestMatchingCycle cycle{traders};

    cycle.wait_for_order(limit_order{Ticker::ETH, buy, 100.0, 10.0});

    auto end = std::chrono::high_resolution_clock::now();
    const int64_t duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double wait_time_ms = TEST_CLIENT_WAIT_SECS * 1000;

    EXPECT_GE(duration_ms, wait_time_ms);
    EXPECT_LE(duration_ms, wait_time_ms + 1000.0);
}

// Disable trader and confirm it doesn't send any orders
TEST_F(IntegrationBasicAlgo, DisableTrader)
{
    auto& trader1 = start_wrappers(traders, "test_algos/buy_tsla_at_100.py");
    auto trader2 = traders.add_trader<TestTrader>(0);
    trader2->modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order({Ticker::ETH, sell, 100.0, 100.0});

    trader1.disable();

    TestMatchingCycle cycle{traders};

    cycle.on_tick(0);
    sleep(1);
    auto orders = trader1.read_orders();
    ASSERT_TRUE(orders.empty());
}
} // namespace nutc::test
