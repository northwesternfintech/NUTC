#include "common/types/messages/messages_wrapper_to_exchange.hpp"
#include "common/util.hpp"
#include "exchange/exchange_state.hpp"
#include "util/helpers/test_cycle.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"
#include "util/process.hpp"

#include <gtest/gtest.h>

namespace nutc::test {
using nutc::common::AlgoLanguage;

class IntegrationBasicCancellation : public ::testing::TestWithParam<AlgoLanguage> {
protected:
    using Ticker = nutc::common::Ticker;
    using nutc::common::Side::buy;
    using nutc::common::Side::sell;
    exchange::exchange_state state_;
};

TEST_P(IntegrationBasicCancellation, CancelMessageHasSameIdAsOrder)
{
    start_wrappers(state_.traders, GetParam(), "cancel_limit_order");
    TestMatchingCycle cycle{state_};

    auto order = cycle.wait_for_order(limit_order{Ticker::ETH, buy, 100.0, 10.0});
    cycle.wait_for_order(common::cancel_order{common::Ticker::ETH, order.order_id});
}

TEST_P(IntegrationBasicCancellation, CancelMessagePreventsOrderFromExecuting)
{
    auto& trader1 = start_wrappers(state_.traders, GetParam(), "cancel_limit_order");
    auto trader2 = state_.traders.add_trader<TestTrader>(0);
    trader2->get_portfolio().modify_holdings(Ticker::ETH, 100.0);
    TestMatchingCycle cycle{state_};

    auto order = cycle.wait_for_order(limit_order{Ticker::ETH, buy, 100.0, 10.0});
    cycle.wait_for_order(common::cancel_order{common::Ticker::ETH, order.order_id});
    trader2->add_order(common::market_order{Ticker::ETH, sell, 10.0});

    cycle.on_tick(0);

    EXPECT_EQ(trader1.get_portfolio().get_capital_delta(), 0);
    EXPECT_EQ(trader2->get_portfolio().get_capital_delta(), 0);
    EXPECT_EQ(trader1.get_portfolio().get_holdings(Ticker::ETH), 0);
}

TEST_P(IntegrationBasicCancellation, OneOfTwoOrdersCancelledResultsInMatch)
{
    auto& trader1 =
        start_wrappers(state_.traders, GetParam(), "partial_cancel_limit_order");
    auto trader2 = state_.traders.add_trader<TestTrader>(0);
    trader2->get_portfolio().modify_holdings(Ticker::ETH, 100.0);
    TestMatchingCycle cycle{state_};

    auto order = cycle.wait_for_order(limit_order{Ticker::ETH, buy, 100.0, 10.0});
    // Assume non-cancelled order got through
    cycle.wait_for_order(common::cancel_order{common::Ticker::ETH, order.order_id});
    trader2->add_order(common::market_order{Ticker::ETH, sell, 10.0});

    cycle.on_tick(0);

    EXPECT_EQ(double{trader1.get_portfolio().get_capital_delta()}, -200);
    EXPECT_EQ(double{trader2->get_portfolio().get_capital_delta()}, 200);
    EXPECT_EQ(double{trader1.get_portfolio().get_holdings(Ticker::ETH)}, 10);
}

INSTANTIATE_TEST_SUITE_P(
    IntegrationBasicCancellation, IntegrationBasicCancellation,
    ::testing::Values(AlgoLanguage::python, AlgoLanguage::cpp)
);
} // namespace nutc::test
