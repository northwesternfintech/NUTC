#include "config.h"
#include "exchange/traders/trader_container.hpp"
#include "util/helpers/test_cycle.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"
#include "util/process.hpp"

#include <gtest/gtest.h>

namespace nutc {
namespace test {
using nutc::test::start_wrappers;
using nutc::util::Side::buy;
using nutc::util::Side::sell;

class IntegrationBasicAlgo : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;

    TraderContainer& users_ =
        nutc::traders::TraderContainer::get_instance(); // NOLINT(*)
};

TEST_F(IntegrationBasicAlgo, InitialLiquidity)
{
    auto trader1 = start_wrappers(users_, "test_algos/buy_tsla_at_100.py");
    auto trader2 = users_.add_trader<TestTrader>(0);
    trader2->modify_holdings("ABC", 1000); // NOLINT
    trader2->add_order({sell, "ABC", 100, 100});

    TestMatchingCycle cycle{
        {"ABC"},
        {trader1, trader2},
    };

    cycle.wait_for_order({buy, "ABC", 100, 10});
}

TEST_F(IntegrationBasicAlgo, RemoveIOCOrder)
{
    auto trader1 = start_wrappers(users_, "test_algos/buy_tsla_at_100.py");
    auto trader2 = users_.add_trader<TestTrader>(0);
    trader2->modify_holdings("ABC", 1000); // NOLINT
    trader2->add_order({sell, "ABC", 100, 100, true});

    TestMatchingCycle cycle{
        {"ABC"},
        {trader1, trader2},
    };

    cycle.on_tick(0);

    cycle.wait_for_order({buy, "ABC", 100, 10});
}

TEST_F(IntegrationBasicAlgo, MarketOrderBuy)
{
    auto trader1 = start_wrappers(users_, "test_algos/buy_market_order_1000.py");
    auto trader2 = users_.add_trader<TestTrader>(0);
    trader2->modify_holdings("ABC", 1000); // NOLINT
    trader2->add_order({sell, "ABC", 1000, 100});

    TestMatchingCycle cycle{
        {"ABC",   "DEF"  },
        {trader1, trader2},
    };

    cycle.wait_for_order({buy, "DEF", 1000, 1});
}

TEST_F(IntegrationBasicAlgo, MarketOrderSell)
{
    auto trader1 = start_wrappers(users_, "test_algos/sell_market_order_1.py");
    trader1->modify_holdings("ABC", 1000); // NOLINT

    auto trader2 = users_.add_trader<TestTrader>(0);
    trader2->add_order({buy, "ABC", 100, 1});

    TestMatchingCycle cycle{
        {"ABC",   "DEF"  },
        {trader1, trader2},
    };

    cycle.wait_for_order({buy, "DEF", 1000, 1});
}

TEST_F(IntegrationBasicAlgo, ManyUpdates)
{
    auto trader1 = start_wrappers(users_, "test_algos/confirm_1000.py");

    auto trader2 = users_.add_trader<TestTrader>(0);
    trader2->modify_holdings("ABC", 100000); // NOLINT

    TestMatchingCycle cycle{
        {"ABC"},
        {trader1, trader2},
    };

    for (double i = 0; i < 100000; i++) {
        trader2->add_order({sell, "ABC", i, 1});
    }

    cycle.on_tick(0);

    cycle.wait_for_order({buy, "ABC", 100, 10});
}

TEST_F(IntegrationBasicAlgo, OnTradeUpdate)
{
    auto trader1 = start_wrappers(users_, "test_algos/buy_tsla_on_trade.py");

    auto trader2 = users_.add_trader<TestTrader>(0);
    trader2->modify_holdings("ABC", 10000); // NOLINT

    TestMatchingCycle cycle{
        {"ABC",   "DEF"  },
        {trader1, trader2},
    };

    trader2->add_order({sell, "ABC", 100, 100});

    cycle.wait_for_order({buy, "ABC", 102, 10});

    cycle.wait_for_order({buy, "DEF", 100, 1});
}

// Sanity check that it goes through the orderbook
TEST_F(IntegrationBasicAlgo, MultipleLevelOrder)
{
    auto trader1 = start_wrappers(users_, "test_algos/buy_tsla_at_100.py");

    auto trader2 = users_.add_trader<TestTrader>(0);
    trader2->modify_holdings("ABC", 1000); // NOLINT

    TestMatchingCycle cycle{
        {"ABC",   "DEF"  },
        {trader1, trader2},
    };

    trader2->add_order({sell, "ABC", 100, 5});
    trader2->add_order({sell, "ABC", 95, 5});

    cycle.wait_for_order({buy, "ABC", 100, 10});
    ASSERT_EQ(trader1->get_capital() - trader1->get_initial_capital(), -975.0);
}

TEST_F(IntegrationBasicAlgo, OnAccountUpdateSell)
{
    auto trader1 = start_wrappers(users_, "test_algos/sell_tsla_on_account.py");
    trader1->modify_holdings("ABC", 1000);

    auto trader2 = users_.add_trader<TestTrader>(100000);
    trader2->add_order({buy, "ABC", 102, 102});

    TestMatchingCycle cycle{
        {"ABC",   "DEF"  },
        {trader1, trader2},
    };

    // obupdate triggers one user to place autil::Side::buy order of 10 ABC at 102
    cycle.wait_for_order({sell, "ABC", 100, 10});

    // on_trade_match triggers one user to place autil::Side::buy order of 1 ABC at
    // 100
    cycle.wait_for_order({buy, "DEF", 100, 1});
}

TEST_F(IntegrationBasicAlgo, OnAccountUpdateBuy)
{
    auto trader1 = start_wrappers(users_, "test_algos/buy_tsla_on_account.py");

    auto trader2 = users_.add_trader<TestTrader>(0);
    trader2->modify_holdings("ABC", 1000); // NOLINT
    trader2->add_order({sell, "ABC", 100, 100});

    TestMatchingCycle cycle{
        {"ABC",   "DEF"  },
        {trader1, trader2},
    };

    // obupdate triggers one user to place autil::Side::buy order of 10 ABC at 102
    cycle.wait_for_order({buy, "ABC", 102, 10});
    // on_trade_match triggers one user to place autil::Side::buy order of 1 ABC at
    // 100
    cycle.wait_for_order({buy, "DEF", 100, 1});
}

TEST_F(IntegrationBasicAlgo, AlgoStartDelay)
{
    auto trader1 = start_wrappers(
        users_, "test_algos/buy_tsla_at_100.py", TEST_STARTING_CAPITAL,
        TEST_CLIENT_WAIT_SECS
    );

    auto start = std::chrono::high_resolution_clock::now();

    auto trader2 = users_.add_trader<TestTrader>(0);
    trader2->modify_holdings("ABC", 1000); // NOLINT
    trader2->add_order({sell, "ABC", 100, 100});

    TestMatchingCycle cycle{
        {"ABC"},
        {trader1, trader2},
    };

    cycle.wait_for_order({buy, "ABC", 100, 10});

    auto end = std::chrono::high_resolution_clock::now();
    const int64_t duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double wait_time_ms = TEST_CLIENT_WAIT_SECS * 1000;

    EXPECT_GE(duration_ms, wait_time_ms);
    EXPECT_LE(duration_ms, wait_time_ms + 1000);
}

// Disable trader and confirm it doesn't send any orders
TEST_F(IntegrationBasicAlgo, DisableTrader)
{
    auto trader1 = start_wrappers(users_, "test_algos/buy_tsla_at_100.py");
    auto trader2 = users_.add_trader<TestTrader>(0);
    trader2->modify_holdings("ABC", 1000); // NOLINT
    trader2->add_order({sell, "ABC", 100, 100});

    traders::TraderContainer::get_instance().remove_trader(trader1);

    TestMatchingCycle cycle{
        {"ABC"},
        {trader1, trader2},
    };

    cycle.on_tick(0);
    sleep(1);
    ASSERT_TRUE(trader1->read_orders().empty());
}
} // namespace test
} // namespace nutc
