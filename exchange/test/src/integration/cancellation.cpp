#include "common/messages_wrapper_to_exchange.hpp"
#include "util/helpers/test_cycle.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"
#include "util/process.hpp"

#include <gtest/gtest.h>

namespace nutc::test {
class IntegrationBasicCancellation : public ::testing::Test {
protected:
    using Ticker = nutc::common::Ticker;
    using nutc::common::Side::buy;
    using nutc::common::Side::sell;
    exchange::TraderContainer traders_;
};

TEST_F(IntegrationBasicCancellation, CancelMessageHasSameIdAsOrder)
{
    start_wrappers(traders_, "test_algos/cancellation/cancel_limit_order.py");
    TestMatchingCycle cycle{traders_};

    auto order_id = cycle.wait_for_order(limit_order{Ticker::ETH, buy, 100.0, 10.0});
    EXPECT_TRUE(order_id.has_value());
    cycle.wait_for_order(common::cancel_order{common::Ticker::ETH, *order_id});
}

TEST_F(IntegrationBasicCancellation, CancelMessagePreventsOrderFromExecuting)
{
    auto& trader1 =
        start_wrappers(traders_, "test_algos/cancellation/cancel_limit_order.py");
    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader2->modify_holdings(Ticker::ETH, 100.0);
    TestMatchingCycle cycle{traders_};

    auto order_id = cycle.wait_for_order(limit_order{Ticker::ETH, buy, 100.0, 10.0});
    EXPECT_TRUE(order_id.has_value());
    cycle.wait_for_order(common::cancel_order{common::Ticker::ETH, *order_id});
    trader2->add_order(common::market_order{Ticker::ETH, sell, 10.0});

    cycle.on_tick(0);

    EXPECT_EQ(trader1.get_capital_delta(), 0);
    EXPECT_EQ(trader2->get_capital_delta(), 0);
    EXPECT_EQ(trader1.get_holdings(Ticker::ETH), 0);
}

TEST_F(IntegrationBasicCancellation, OneOfTwoOrdersCancelledResultsInMatch)
{
    auto& trader1 = start_wrappers(
        traders_, "test_algos/cancellation/partial_cancel_limit_order.py"
    );
    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader2->modify_holdings(Ticker::ETH, 100.0);
    TestMatchingCycle cycle{traders_};

    auto order_id = cycle.wait_for_order(limit_order{Ticker::ETH, buy, 100.0, 10.0});
    // Assume non-cancelled order got through
    EXPECT_TRUE(order_id.has_value());
    cycle.wait_for_order(common::cancel_order{common::Ticker::ETH, *order_id});
    trader2->add_order(common::market_order{Ticker::ETH, sell, 10.0});

    cycle.on_tick(0);

    EXPECT_EQ(double{trader1.get_capital_delta()}, -200);
    EXPECT_EQ(double{trader2->get_capital_delta()}, 200);
    EXPECT_EQ(double{trader1.get_holdings(Ticker::ETH)}, 10);
}
} // namespace nutc::test
