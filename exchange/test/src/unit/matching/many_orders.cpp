#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitManyOrders : public ::testing::Test {
protected:
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        manager_.add_local_trader("A");
        manager_.add_local_trader("B");
        manager_.add_local_trader("C");
        manager_.add_local_trader("D");

        manager_.get_trader("A")->modify_capital(STARTING_CAPITAL);
        manager_.get_trader("B")->modify_capital(STARTING_CAPITAL);
        manager_.get_trader("C")->modify_capital(STARTING_CAPITAL);
        manager_.get_trader("D")->modify_capital(STARTING_CAPITAL);

        manager_.get_trader("A")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("B")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("C")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("D")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    }

    ClientManager& manager_ = nutc::manager::ClientManager::get_instance(); // NOLINT(*)
    Engine engine_{}; // NOLINT (*)

    nutc::matching::match_result_t
    add_to_engine_(MarketOrder order)
    {
        return engine_.match_order(std::move(order), manager_);
    }
};

TEST_F(UnitManyOrders, CorrectTimePriority)
{
    MarketOrder order1{"A", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"B", BUY, "ETHUSD", 1, 1};
    MarketOrder order3{"C", SELL, "ETHUSD", 1, 1};
    MarketOrder order4{"D", BUY, "ETHUSD", 1, 1};

    auto [matches1, updates1] = add_to_engine_(order1);
    auto [matches2, updates2] = add_to_engine_(order2);
    engine_.add_order(order3);
    auto [matches3, updates3] = add_to_engine_(order4);
    ASSERT_EQ(matches1.size(), 0);
    ASSERT_EQ(updates1.size(), 1);
    ASSERT_EQ_OB_UPDATE(updates1[0], "ETHUSD", BUY, 1, 1);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(updates2.size(), 1);
    ASSERT_EQ_OB_UPDATE(updates2[0], "ETHUSD", BUY, 1, 1);

    ASSERT_EQ(matches3.size(), 1);
    ASSERT_EQ(updates3.size(), 3);
    ASSERT_EQ_OB_UPDATE(updates3[0], "ETHUSD", BUY, 1, 0);
    ASSERT_EQ_OB_UPDATE(updates3[1], "ETHUSD", SELL, 1, 0);
    ASSERT_EQ_OB_UPDATE(updates3[2], "ETHUSD", BUY, 1, 1);
    ASSERT_EQ_MATCH(matches3[0], "ETHUSD", "A", "C", SELL, 1, 1);
}

TEST_F(UnitManyOrders, OnlyMatchesOne)
{
    MarketOrder order1{"A", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"B", SELL, "ETHUSD", 1, 1};

    auto [matches1, updates1] = add_to_engine_(order1);
    auto [matches2, updates2] = add_to_engine_(order1);
    ASSERT_EQ(matches1.size(), 0);
    ASSERT_EQ(updates1.size(), 1);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(updates2.size(), 1);

    auto [matches3, updates3] = add_to_engine_(order2);
    ASSERT_EQ(matches3.size(), 1);
    ASSERT_EQ(updates3.size(), 1);
    ASSERT_EQ_MATCH(matches3[0], "ETHUSD", "A", "B", SELL, 1, 1);
    ASSERT_EQ_OB_UPDATE(updates3[0], "ETHUSD", BUY, 1, 0);
}

TEST_F(UnitManyOrders, SimpleManyOrder)
{
    MarketOrder order1{"A", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"B", BUY, "ETHUSD", 1, 1};
    MarketOrder order3{"C", BUY, "ETHUSD", 1, 1};
    MarketOrder order4{"D", SELL, "ETHUSD", 3, 1};

    auto [matches1, updates1] = add_to_engine_(order1);
    auto [matches2, updates2] = add_to_engine_(order2);
    auto [matches3, updates3] = add_to_engine_(order3);

    ASSERT_EQ(matches1.size(), 0);
    ASSERT_EQ(updates1.size(), 1);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(updates2.size(), 1);
    ASSERT_EQ(matches3.size(), 0);
    ASSERT_EQ(updates3.size(), 1);

    auto [matches4, updates4] = add_to_engine_(order4);
    ASSERT_EQ(matches4.size(), 3);
    ASSERT_EQ(updates4.size(), 3);

    ASSERT_EQ_MATCH(matches4[0], "ETHUSD", "A", "D", SELL, 1, 1);
    ASSERT_EQ_MATCH(matches4[1], "ETHUSD", "B", "D", SELL, 1, 1);
    ASSERT_EQ_MATCH(matches4[2], "ETHUSD", "C", "D", SELL, 1, 1);

    ASSERT_EQ_OB_UPDATE(updates4[0], "ETHUSD", BUY, 1, 0);
    ASSERT_EQ_OB_UPDATE(updates4[1], "ETHUSD", BUY, 1, 0);
    ASSERT_EQ_OB_UPDATE(updates4[2], "ETHUSD", BUY, 1, 0);
}

TEST_F(UnitManyOrders, PassiveAndAggressivePartial)
{
    MarketOrder order1{"A", SELL, "ETHUSD", 1, 1};
    MarketOrder order2{"B", SELL, "ETHUSD", 10, 1}; // NOLINT (*)
    MarketOrder order3{"C", BUY, "ETHUSD", 2, 3};
    MarketOrder order4{"D", BUY, "ETHUSD", 10, 4}; // NOLINT (*)

    auto [matches1, updates1] = add_to_engine_(order1);
    auto [matches2, updates2] = add_to_engine_(order2);
    auto [matches3, updates3] = add_to_engine_(order3);
    auto [matches4, updates4] = add_to_engine_(order4);

    ASSERT_EQ(matches1.size(), 0);
    ASSERT_EQ(updates1.size(), 1);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(updates2.size(), 1);
    ASSERT_EQ(matches3.size(), 2);
    ASSERT_EQ(updates3.size(), 3);
    ASSERT_EQ(matches4.size(), 1);
    ASSERT_EQ(updates4.size(), 2);

    ASSERT_EQ_MATCH(matches3[0], "ETHUSD", "C", "A", BUY, 1, 1);
    ASSERT_EQ_MATCH(matches3[1], "ETHUSD", "C", "B", BUY, 1, 1);
    ASSERT_EQ_OB_UPDATE(updates3[0], "ETHUSD", SELL, 1, 0);
    ASSERT_EQ_OB_UPDATE(updates3[1], "ETHUSD", SELL, 1, 0);
    ASSERT_EQ_OB_UPDATE(updates3[2], "ETHUSD", SELL, 1, 9);

    ASSERT_EQ_MATCH(matches4[0], "ETHUSD", "D", "B", BUY, 1, 9);
    ASSERT_EQ_OB_UPDATE(updates4[0], "ETHUSD", SELL, 1, 0);
    ASSERT_EQ_OB_UPDATE(updates4[1], "ETHUSD", BUY, 4, 1);
}
