#include "exchange/traders/trader_container.hpp"

#include "config.h"
#include "util/helpers/test_trader.hpp"

#include <gtest/gtest.h>

using namespace nutc;

struct control_block {
    std::atomic<size_t> strong_ref_count;
    std::atomic<size_t> weak_ref_count;
    void (*deleter)(void*);
    void* allocator;
};

class UnitTraderContainerTest : public ::testing::Test {
protected:
    exchange::TraderContainer traders;
};

TEST_F(UnitTraderContainerTest, PointersUseSharedMemory)
{
    auto trader1 = traders.add_trader<test::TestTrader>("ABC", TEST_STARTING_CAPITAL);
    auto trader2 = traders.add_trader<test::TestTrader>("DEF", TEST_STARTING_CAPITAL);
    auto trader3 = traders.add_trader<test::TestTrader>("GHI", TEST_STARTING_CAPITAL);

#ifdef __linux__
    std::ptrdiff_t distance1 =
        reinterpret_cast<char*>(trader2.get()) - reinterpret_cast<char*>(trader1.get());
    std::ptrdiff_t distance2 =
        reinterpret_cast<char*>(trader3.get()) - reinterpret_cast<char*>(trader2.get());
#elifdef __APPLE__
    std::ptrdiff_t distance1 =
        reinterpret_cast<char*>(trader1.get()) - reinterpret_cast<char*>(trader2.get());
    std::ptrdiff_t distance2 =
        reinterpret_cast<char*>(trader2.get()) - reinterpret_cast<char*>(trader3.get());
#endif

    static constexpr auto EXPECTED_SIZE =
        sizeof(test::TestTrader) + sizeof(control_block);
    EXPECT_LE(distance1, EXPECTED_SIZE);
    EXPECT_LE(distance2, EXPECTED_SIZE);
}
