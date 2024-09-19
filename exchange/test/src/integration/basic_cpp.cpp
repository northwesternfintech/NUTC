#include "common/util.hpp"
#include "util/helpers/compilation.hpp"
#include "util/helpers/test_cycle.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/process.hpp"

#include <gtest/gtest.h>

namespace nutc::test {
using common::limit_order;
using nutc::common::AlgoLanguage;
using nutc::common::Ticker;
using nutc::common::Side::buy;
using nutc::common::Side::sell;

class IntegrationBasicCpp : public ::testing::Test {
protected:
    exchange::TraderContainer traders_;
};

TEST_F(IntegrationBasicCpp, CppAlgo)
{
    testing::compile_cpp("buy_eth");
    start_wrappers(traders_, {AlgoLanguage::cpp, "test_algos/cpp/buy_eth.so"});
    TestMatchingCycle cycle{traders_};

    cycle.wait_for_order(limit_order{Ticker::ETH, buy, 100.0, 10.0});
}

TEST_F(IntegrationBasicCpp, ConfirmOrderReceived)
{
    testing::compile_cpp("buy_tsla_at_100");
    start_wrappers(
        traders_, {AlgoLanguage::cpp, "test_algos/cpp/buy_tsla_at_100.so"}
    );
    auto trader2 = traders_.add_trader<TestTrader>(0);
    trader2->modify_holdings(Ticker::ETH, 1000.0); // NOLINT
    trader2->add_order(limit_order{Ticker::ETH, sell, 100.0, 10.0});

    TestMatchingCycle cycle{traders_};

    cycle.wait_for_order(limit_order{Ticker::ETH, buy, 100.0, 10.0});
    ASSERT_EQ(
        double{trader2->get_capital() - trader2->get_initial_capital()}, 100.0 * 10.0
    );
}
} // namespace nutc::test
