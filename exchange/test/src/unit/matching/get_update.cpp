#include "config.h"
#include "exchange/tickers/engine/level_update_generator.hpp"
#include "exchange/traders/trader_container.hpp"
#include "shared/util.hpp"
#include "test_utils/helpers/test_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

#include <algorithm>

using nutc::matching::get_updates;
using nutc::matching::OrderBook;
using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitGetUpdate : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;
    std::shared_ptr<nutc::traders::GenericTrader> trader1, trader2, trader3;

    void
    SetUp() override
    {
        trader1 =
            manager_.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
        trader2 =
            manager_.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);
        trader3 =
            manager_.add_trader<TestTrader>(std::string("GHI"), TEST_STARTING_CAPITAL);
    }

    TraderContainer& manager_ =
        nutc::traders::TraderContainer::get_instance(); // NOLINT(*)

    OrderBook before{}; // NOLINT (*)
    OrderBook after{};  // NOLINT (*)
};

TEST_F(UnitGetUpdate, NoOrders)
{
    auto updates = get_updates("A", before, after);

    ASSERT_EQ(updates.size(), 0);
}

TEST_F(UnitGetUpdate, OrderAdded)
{
    stored_order order1{trader1, buy, "A", 1, 1, 0};

    after.add_order(order1);

    auto updates = get_updates("A", before, after);

    ASSERT_EQ(updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(updates[0], "A", buy, 1, 1);
}

TEST_F(UnitGetUpdate, OrderDeleted)
{
    stored_order order1{trader1, buy, "A", 1, 1, 0};

    before.add_order(order1);

    auto updates = get_updates("A", before, after);

    ASSERT_EQ(updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(updates[0], "A", buy, 1, 0);
}

TEST_F(UnitGetUpdate, OrderQuantityChange)
{
    stored_order order1{trader1, buy, "A", 1, 1, 0};
    before.add_order(order1);

    stored_order order2{trader1, buy, "A", 5, 1, 0};
    after.add_order(order2);

    auto updates = get_updates("A", before, after);

    ASSERT_EQ(updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(updates[0], "A", buy, 1, 5);
}

TEST_F(UnitGetUpdate, NoQuanitityChange)
{
    stored_order order1{trader1, buy, "A", 1, 1, 0};
    stored_order order2{trader1, buy, "A", 1, 2, 0};
    stored_order order3{trader1, buy, "A", 1, 3, 0};
    before.add_order(order1);
    before.add_order(order2);
    before.add_order(order3);

    stored_order order4{trader1, buy, "A", 1, 3, 0};
    stored_order order5{trader1, buy, "A", 1, 2, 0};
    stored_order order6{trader1, buy, "A", 1, 1, 0};
    after.add_order(order4);
    after.add_order(order5);
    after.add_order(order6);

    auto updates = get_updates("A", before, after);

    ASSERT_EQ(updates.size(), 0);
}

TEST_F(UnitGetUpdate, BuySellChange)
{
    stored_order order1{trader1, buy, "A", 1, 1, 0};
    stored_order order2{trader3, sell, "A", 1, 5, 0};
    before.add_order(order1);
    before.add_order(order2);

    stored_order order3{trader2, buy, "A", 5, 1, 0};
    stored_order order4{trader1, sell, "A", 5, 5, 0};
    after.add_order(order3);
    after.add_order(order4);

    auto updates = get_updates("A", before, after);

    std::sort(updates.begin(), updates.end(), [](auto a, auto b) {
        return a.price < b.price;
    });

    ASSERT_EQ(updates.size(), 2);
    ASSERT_EQ_OB_UPDATE(updates[0], "A", buy, 1, 5);
    ASSERT_EQ_OB_UPDATE(updates[1], "A", sell, 5, 5);
}

TEST_F(UnitGetUpdate, ManyLevelChanges)
{
    stored_order order1{trader1, buy, "A", 1, 1, 0};
    stored_order order2{trader1, buy, "A", 1, 2, 0};
    stored_order order3{trader1, buy, "A", 1, 3, 0};
    stored_order order4{trader3, buy, "A", 1, 4, 0};
    before.add_order(order1);
    before.add_order(order2);
    before.add_order(order3);
    before.add_order(order4);

    stored_order order5{trader1, buy, "A", 9, 2, 0};
    stored_order order6{trader1, buy, "A", 1, 1, 0};
    stored_order order7{trader1, buy, "A", 10, 3, 0};
    stored_order order8{trader1, buy, "A", 8, 4, 0};
    after.add_order(order5);
    after.add_order(order6);
    after.add_order(order7);
    after.add_order(order8);

    auto updates = get_updates("A", before, after);

    std::sort(updates.begin(), updates.end(), [](auto a, auto b) {
        return a.price < b.price;
    });

    ASSERT_EQ(updates.size(), 3);
    ASSERT_EQ_OB_UPDATE(updates[0], "A", buy, 2, 9);
    ASSERT_EQ_OB_UPDATE(updates[1], "A", buy, 3, 10);
    ASSERT_EQ_OB_UPDATE(updates[2], "A", buy, 4, 8);
}

TEST_F(UnitGetUpdate, ChangesAddsAndDeletes)
{
    stored_order order1{trader1, buy, "A", 1, 1, 0};
    stored_order order2{trader1, buy, "A", 1, 2, 0};
    stored_order order3{trader1, buy, "A", 1, 3, 0};
    stored_order order4{trader3, buy, "A", 1, 4, 0};
    stored_order order5{trader3, buy, "A", 1, 5, 0};
    before.add_order(order1);
    before.add_order(order2);
    before.add_order(order3);
    before.add_order(order4);
    before.add_order(order5);

    stored_order order6{trader1, buy, "A", 9, 2, 0};
    stored_order order7{trader1, buy, "A", 1, 1, 0};
    stored_order order8{trader1, buy, "A", 10, 3, 0};
    stored_order order9{trader1, buy, "A", 8, 4, 0};
    stored_order order10{trader1, buy, "A", 2, 10, 0};
    after.add_order(order6);
    after.add_order(order7);
    after.add_order(order8);
    after.add_order(order9);
    after.add_order(order10);

    auto updates = get_updates("A", before, after);

    std::sort(updates.begin(), updates.end(), [](auto a, auto b) {
        return a.price < b.price;
    });

    ASSERT_EQ(updates.size(), 5);
    ASSERT_EQ_OB_UPDATE(updates[0], "A", buy, 2, 9);
    ASSERT_EQ_OB_UPDATE(updates[1], "A", buy, 3, 10);
    ASSERT_EQ_OB_UPDATE(updates[2], "A", buy, 4, 8);
    ASSERT_EQ_OB_UPDATE(updates[3], "A", buy, 5, 0);
    ASSERT_EQ_OB_UPDATE(updates[4], "A", buy, 10, 2);
}
