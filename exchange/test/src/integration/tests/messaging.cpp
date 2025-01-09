#include "common/util.hpp"
#include "exchange/exchange_state.hpp"
#include "util/helpers/test_cycle.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"
#include "util/process.hpp"

#include <gtest/gtest.h>
#include <unistd.h>

namespace nutc::test {
using namespace nutc::common;

class IntegrationMessaging : public ::testing::TestWithParam<AlgoLanguage> {
protected:
    exchange::exchange_state state_;
};

TEST_P(IntegrationMessaging, ConfirmGeneratedOrderIdIsNotSamePID)
{
    start_wrappers(state_.traders, GetParam(), "buy_tsla_at_100");
    auto trader2 = state_.traders.add_trader<TestTrader>(0);
    trader2->get_portfolio().modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order(limit_order{Ticker::ETH, Side::sell, 100.0, 10.0});

    TestMatchingCycle cycle{state_};

    auto order = cycle.wait_for_order(limit_order{Ticker::ETH, Side::buy, 100.0, 10.0});

    auto remote_pid = (order.order_id >> 46) & 0xFF;
    auto local_pid = (generate_order_id() >> 46) & 0xFF;
    EXPECT_NE(remote_pid, local_pid);
}

TEST_P(
    IntegrationMessaging, ConfirmOrderTimestampIsNotOverwrittenByExchangeWhenReceived
)
{
    start_wrappers(state_.traders, GetParam(), "buy_eth");

    usleep(50000);

    auto start = get_time();

    TestMatchingCycle cycle{state_};

    auto order = cycle.wait_for_order(limit_order{Ticker::ETH, Side::buy, 100.0, 10.0});

    EXPECT_LT(order.timestamp, start);
}

INSTANTIATE_TEST_SUITE_P(
    IntegrationMessaging, IntegrationMessaging,
    ::testing::Values(AlgoLanguage::python, AlgoLanguage::cpp)
);
} // namespace nutc::test
