
#include "common/messages_wrapper_to_exchange.hpp"
#include "util/helpers/test_cycle.hpp"
#include "util/macros.hpp"
#include "util/process.hpp"

#include <gtest/gtest.h>

namespace nutc::test {
class IntegrationBasicCancellation : public ::testing::Test {
protected:
    using Ticker = nutc::common::Ticker;
    using nutc::common::Side::buy;
    using nutc::common::Side::sell;
    exchange::TraderContainer traders;
};

TEST_F(IntegrationBasicCancellation, BasicSendCancelAfterOrder)
{
    start_wrappers(traders, "test_algos/cancel_limit_order.py");
    TestMatchingCycle cycle{traders};

    auto order_id = cycle.wait_for_order(limit_order{Ticker::ETH, buy, 100.0, 10.0});
    EXPECT_TRUE(order_id.has_value());
    cycle.wait_for_order(common::cancel_order{common::Ticker::ETH, *order_id});
}

} // namespace nutc::test
