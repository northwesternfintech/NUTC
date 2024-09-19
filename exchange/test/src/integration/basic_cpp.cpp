#include "common/util.hpp"
#include "util/helpers/compilation.hpp"
#include "util/helpers/test_cycle.hpp"
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
    ASSERT_TRUE(1 == 1);
}
} // namespace nutc::test
