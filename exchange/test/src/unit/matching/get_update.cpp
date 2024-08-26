#include "config.h"
#include "exchange/orders/level_tracking/level_update_generator.hpp"
#include "exchange/orders/orderbook/level_tracked_orderbook.hpp"
#include "shared/util.hpp"
#include "util/helpers/test_trader.hpp"
#include "util/macros.hpp"

#include <gtest/gtest.h>

#include <algorithm>

using nutc::exchange::LevelTrackedOrderbook;
using nutc::exchange::LevelUpdateGenerator;
using nutc::exchange::LimitOrderBook;
using nutc::shared::Ticker;
using nutc::shared::Side::buy;
using nutc::shared::Side::sell;

class UnitGetUpdate : public ::testing::Test {
protected:
    using TestTrader = nutc::test::TestTrader;
    TraderContainer traders;

    nutc::exchange::GenericTrader& trader1 =
        *traders.add_trader<TestTrader>(std::string("ABC"), TEST_STARTING_CAPITAL);
    nutc::exchange::GenericTrader& trader2 =
        *traders.add_trader<TestTrader>(std::string("DEF"), TEST_STARTING_CAPITAL);
    nutc::exchange::GenericTrader& trader3 =
        *traders.add_trader<TestTrader>(std::string("GHI"), TEST_STARTING_CAPITAL);

    LevelTrackedOrderbook<LimitOrderBook> ob{};
    LevelUpdateGenerator& generator_ = ob.get_update_generator();
};

TEST_F(UnitGetUpdate, NoOrders)
{
    // auto updates = get_updates(Ticker::ETH, before, after);
    auto updates = generator_.get_updates(Ticker::ETH);

    ASSERT_EQ(updates.size(), 0);
}

TEST_F(UnitGetUpdate, OrderAdded)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0};

    ob.add_order(order1);

    auto updates = generator_.get_updates(Ticker::ETH);

    ASSERT_EQ(updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 1, 1);
}

TEST_F(UnitGetUpdate, OrderDeleted)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0};

    // before, we have a single order
    ob.add_order(order1);

    generator_.reset();

    // we delete the order

    ob.mark_order_removed(order1);
    //
    auto updates = generator_.get_updates(Ticker::ETH);

    ASSERT_EQ(updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 0, 1);
}

TEST_F(UnitGetUpdate, OrderQuantityChange)
{
    double initial_quantity = 1;
    tagged_limit_order order1{trader1, Ticker::ETH, buy, initial_quantity, 1.0};
    ob.add_order(order1);

    double quantity_delta = 5;
    ob.change_quantity(order1, quantity_delta);
    auto updates = generator_.get_updates(Ticker::ETH);

    ASSERT_EQ(updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(
        updates[0], Ticker::ETH, buy, quantity_delta + initial_quantity, 1
    );
}

// This is an edge case that we currently level change updator doesn't handle
/* TEST_F(UnitGetUpdate, NoQuanitityChange)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1, 0};
    tagged_limit_order order2{trader1, Ticker::ETH, buy, 1, 2, 0};
    tagged_limit_order order3{trader1, Ticker::ETH, buy, 1, 3, 0};
    before.add_order(order1);
    before.add_order(order2);
    before.add_order(order3);

    tagged_limit_order order4{trader1, Ticker::ETH, buy, 1, 3, 0};
    tagged_limit_order order5{trader1, Ticker::ETH, buy, 1, 2, 0};
    tagged_limit_order order6{trader1, Ticker::ETH, buy, 1, 1, 0};
    after.add_order(order4);
    after.add_order(order5);
    after.add_order(order6);

    auto updates = get_updates(Ticker::ETH, before, after);

    ASSERT_EQ(updates.size(), 0);
} */

TEST_F(UnitGetUpdate, BuySellChange)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0};
    tagged_limit_order order2{trader3, Ticker::ETH, sell, 1, 5.0};

    ob.add_order(order1);
    ob.add_order(order2);

    ob.change_quantity(order1, 4);
    ob.change_quantity(order2, 4);

    auto updates = generator_.get_updates(Ticker::ETH);

    std::sort(updates.begin(), updates.end(), [](auto a, auto b) {
        return a.price < b.price;
    });

    ASSERT_EQ(updates.size(), 2);
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 5, 1);
    ASSERT_EQ_OB_UPDATE(updates[1], Ticker::ETH, sell, 5, 5);
}

TEST_F(UnitGetUpdate, ManyLevelChanges)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0};
    tagged_limit_order order2{trader1, Ticker::ETH, buy, 1, 2.0};
    tagged_limit_order order3{trader1, Ticker::ETH, buy, 1, 3.0};
    tagged_limit_order order4{trader3, Ticker::ETH, buy, 1, 4.0};

    ob.add_order(order1);
    ob.add_order(order2);
    ob.add_order(order3);
    ob.add_order(order4);

    generator_.reset();

    tagged_limit_order order5{trader1, Ticker::ETH, buy, 8, 2.0};
    tagged_limit_order order6{trader1, Ticker::ETH, buy, 9, 3.0};
    tagged_limit_order order7{trader1, Ticker::ETH, buy, 7, 4.0};

    ob.add_order(order5);
    ob.add_order(order6);
    ob.add_order(order7);
    auto updates = generator_.get_updates(Ticker::ETH);

    std::sort(updates.begin(), updates.end(), [](auto a, auto b) {
        return a.price < b.price;
    });

    ASSERT_EQ(updates.size(), 3);
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 9, 2);
    ASSERT_EQ_OB_UPDATE(updates[1], Ticker::ETH, buy, 10, 3);
    ASSERT_EQ_OB_UPDATE(updates[2], Ticker::ETH, buy, 8, 4);
}

TEST_F(UnitGetUpdate, ChangesAddsAndDeletes)
{
    tagged_limit_order order1{trader1, Ticker::ETH, buy, 1, 1.0};
    tagged_limit_order order2{trader1, Ticker::ETH, buy, 1, 2.0};
    tagged_limit_order order3{trader1, Ticker::ETH, buy, 1, 3.0};
    tagged_limit_order order4{trader3, Ticker::ETH, buy, 10, 4.0};
    tagged_limit_order order5{trader3, Ticker::ETH, buy, 5, 5.0};
    tagged_limit_order order6{trader3, Ticker::ETH, buy, 5, 5.0};

    ob.add_order(order1);
    ob.add_order(order2);
    ob.add_order(order3);
    ob.add_order(order4);
    ob.add_order(order5);
    ob.add_order(order6);

    generator_.reset();

    tagged_limit_order order7{trader1, Ticker::ETH, buy, 8, 2.0};
    tagged_limit_order order8{trader1, Ticker::ETH, buy, 9, 3.0};

    ob.add_order(order7);
    ob.add_order(order8);

    ob.mark_order_removed(order6);

    auto updates = generator_.get_updates(Ticker::ETH);

    std::sort(updates.begin(), updates.end(), [](auto a, auto b) {
        return a.price < b.price;
    });

    ASSERT_EQ(updates.size(), 3);
    ASSERT_EQ_OB_UPDATE(updates[0], Ticker::ETH, buy, 9, 2);
    ASSERT_EQ_OB_UPDATE(updates[1], Ticker::ETH, buy, 10, 3);
    ASSERT_EQ_OB_UPDATE(updates[2], Ticker::ETH, buy, 5, 5);
}
