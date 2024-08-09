#include "config.h"
#include "exchange/orders/level_tracking/level_update_generator.hpp"
#include "exchange/orders/orderbook/level_tracked_orderbook.hpp"
#include "exchange/traders/trader_container.hpp"
#include "shared/util.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>

using nutc::matching::LevelTrackedOrderbook;
using nutc::matching::LevelUpdateGenerator;
using nutc::matching::LimitOrderBook;
using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitGetUpdate : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;
    TraderContainer manager_;

    nutc::traders::GenericTrader& trader1 =
        *manager_.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader2 =
        *manager_.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);
    nutc::traders::GenericTrader& trader3 =
        *manager_.add_trader<TestTrader>(std::string("GHI"), TEST_STARTING_CAPITAL);

    LevelTrackedOrderbook<LimitOrderBook> ob{};
    LevelUpdateGenerator& generator_ = ob.get_update_generator();
};

TEST_F(UnitGetUpdate, NoOrders)
{
    // auto updates = get_updates("ABC", before, after);
    auto updates = generator_.get_updates("ABC");

    ASSERT_EQ(updates.size(), 0);
}

TEST_F(UnitGetUpdate, OrderAdded)
{
    stored_order order1{trader1, "ABC", buy, 1, 1, 0};

    ob.add_order(order1);

    auto updates = generator_.get_updates("ABC");

    ASSERT_EQ(updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(updates[0], "ABC", buy, 1, 1);
}

TEST_F(UnitGetUpdate, OrderDeleted)
{
    stored_order order1{trader1, "ABC", buy, 1, 1, 0};

    // before, we have a single order
    ob.add_order(order1);

    generator_.reset();

    // we delete the order

    ob.mark_order_removed(order1);
    //
    auto updates = generator_.get_updates("ABC");

    ASSERT_EQ(updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(updates[0], "ABC", buy, 1, 0);
}

TEST_F(UnitGetUpdate, OrderQuantityChange)
{
    double initial_quantity = 1;
    stored_order order1{trader1, "ABC", buy, initial_quantity, 1, 0};
    ob.add_order(order1);

    double quantity_delta = 5;
    ob.change_quantity(order1, quantity_delta);
    auto updates = generator_.get_updates("ABC");

    ASSERT_EQ(updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(updates[0], "ABC", buy, 1, quantity_delta + initial_quantity);
}

// This is an edge case that we currently level change updator doesn't handle
/* TEST_F(UnitGetUpdate, NoQuanitityChange)
{
    stored_order order1{trader1, "ABC", buy, 1, 1, 0};
    stored_order order2{trader1, "ABC", buy, 1, 2, 0};
    stored_order order3{trader1, "ABC", buy, 1, 3, 0};
    before.add_order(order1);
    before.add_order(order2);
    before.add_order(order3);

    stored_order order4{trader1, "ABC", buy, 1, 3, 0};
    stored_order order5{trader1, "ABC", buy, 1, 2, 0};
    stored_order order6{trader1, "ABC", buy, 1, 1, 0};
    after.add_order(order4);
    after.add_order(order5);
    after.add_order(order6);

    auto updates = get_updates("ABC", before, after);

    ASSERT_EQ(updates.size(), 0);
} */

TEST_F(UnitGetUpdate, BuySellChange)
{
    stored_order order1{trader1, "ABC", buy, 1, 1, 0};
    stored_order order2{trader3, "ABC", sell, 5, 1, 0};

    ob.add_order(order1);
    ob.add_order(order2);

    ob.change_quantity(order1, 4);
    ob.change_quantity(order2, 4);

    auto updates = generator_.get_updates("ABC");

    std::sort(updates.begin(), updates.end(), [](auto a, auto b) {
        return a.price < b.price;
    });

    ASSERT_EQ(updates.size(), 2);
    ASSERT_EQ_OB_UPDATE(updates[0], "ABC", buy, 1, 5);
    ASSERT_EQ_OB_UPDATE(updates[1], "ABC", sell, 5, 5);
}

TEST_F(UnitGetUpdate, ManyLevelChanges)
{
    stored_order order1{trader1, "ABC", buy, 1, 1, 0};
    stored_order order2{trader1, "ABC", buy, 2, 1, 0};
    stored_order order3{trader1, "ABC", buy, 3, 1, 0};
    stored_order order4{trader3, "ABC", buy, 4, 1, 0};

    ob.add_order(order1);
    ob.add_order(order2);
    ob.add_order(order3);
    ob.add_order(order4);

    generator_.reset();

    stored_order order5{trader1, "ABC", buy, 2, 8, 0};
    stored_order order6{trader1, "ABC", buy, 3, 9, 0};
    stored_order order7{trader1, "ABC", buy, 4, 7, 0};

    ob.add_order(order5);
    ob.add_order(order6);
    ob.add_order(order7);
    auto updates = generator_.get_updates("ABC");

    std::sort(updates.begin(), updates.end(), [](auto a, auto b) {
        return a.price < b.price;
    });

    ASSERT_EQ(updates.size(), 3);
    ASSERT_EQ_OB_UPDATE(updates[0], "ABC", buy, 2, 9);
    ASSERT_EQ_OB_UPDATE(updates[1], "ABC", buy, 3, 10);
    ASSERT_EQ_OB_UPDATE(updates[2], "ABC", buy, 4, 8);
}

TEST_F(UnitGetUpdate, ChangesAddsAndDeletes)
{
    stored_order order1{trader1, "ABC", buy, 1, 1, 0};
    stored_order order2{trader1, "ABC", buy, 2, 1, 0};
    stored_order order3{trader1, "ABC", buy, 3, 1, 0};
    stored_order order4{trader3, "ABC", buy, 4, 10, 0};
    stored_order order5{trader3, "ABC", buy, 5, 5, 0};
    stored_order order6{trader3, "ABC", buy, 5, 5, 0};

    ob.add_order(order1);
    ob.add_order(order2);
    ob.add_order(order3);
    ob.add_order(order4);
    ob.add_order(order5);
    ob.add_order(order6);

    generator_.reset();

    stored_order order7{trader1, "ABC", buy, 2, 8, 0};
    stored_order order8{trader1, "ABC", buy, 3, 9, 0};

    ob.add_order(order7);
    ob.add_order(order8);

    ob.mark_order_removed(order6);

    auto updates = generator_.get_updates("ABC");

    std::sort(updates.begin(), updates.end(), [](auto a, auto b) {
        return a.price < b.price;
    });

    ASSERT_EQ(updates.size(), 3);
    ASSERT_EQ_OB_UPDATE(updates[0], "ABC", buy, 2, 9);
    ASSERT_EQ_OB_UPDATE(updates[1], "ABC", buy, 3, 10);
    ASSERT_EQ_OB_UPDATE(updates[2], "ABC", buy, 5, 5);
}
