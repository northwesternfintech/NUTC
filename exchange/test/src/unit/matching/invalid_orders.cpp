#include "exchange/config.h"
#include "exchange/tickers/engine/engine.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitInvalidOrders : public ::testing::Test {
protected:
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        using nutc::testing_utils::add_client_simple;
        using nutc::testing_utils::modify_holdings_simple;

        add_client_simple(manager_, "ABC");
        add_client_simple(manager_, "DEF");

        modify_holdings_simple(manager_, "ABC", "ETHUSD", DEFAULT_QUANTITY);
        modify_holdings_simple(manager_, "DEF", "ETHUSD", DEFAULT_QUANTITY);
    }

    ClientManager& manager_ = nutc::manager::ClientManager::get_instance(); // NOLINT(*)
    Engine engine_{}; // NOLINT (*)

    nutc::matching::match_result_t
    add_to_engine_(MarketOrder order)
    {
        return engine_.match_order(std::move(order), manager_);
    }
};

TEST_F(UnitInvalidOrders, SimpleInvalidFunds)
{
    nutc::testing_utils::modify_capital_simple(manager_, "ABC", -STARTING_CAPITAL);
    std::optional<SIDE> err =
        manager_.validate_match(Match{"ETHUSD", SELL, 1, 1, "ABC", "DEF"});

    if (err.has_value())
        ASSERT_EQ(err.value(), BUY);
    else
        FAIL() << "Match should have failed";
}

TEST_F(UnitInvalidOrders, RemoveThenAddFunds)
{
    nutc::testing_utils::modify_capital_simple(manager_, "ABC", -STARTING_CAPITAL);

    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};

    // Thrown out
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 0);

    // Kept, but not matched
    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2[0], "ETHUSD", SELL, 1, 1);

    nutc::testing_utils::modify_capital_simple(manager_, "ABC", STARTING_CAPITAL);

    // Kept, but not matched
    auto [matches3, ob_updates3] = add_to_engine_(order2);
    ASSERT_EQ(matches3.size(), 0);
    ASSERT_EQ(ob_updates3.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates3[0], "ETHUSD", SELL, 1, 1);

    // Kept and matched
    auto [matches4, ob_updates4] = add_to_engine_(order1);
    ASSERT_EQ(matches4.size(), 1);
    ASSERT_EQ(ob_updates4.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates4[0], "ETHUSD", SELL, 1, 0);
    ASSERT_EQ_MATCH(matches4.at(0), "ETHUSD", "ABC", "DEF", BUY, 1, 1);
}

TEST_F(UnitInvalidOrders, MatchingInvalidFunds)
{
    nutc::testing_utils::modify_capital_simple(manager_, "ABC", -STARTING_CAPITAL);

    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};

    // Thrown out
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 0);

    // Kept, but not matched
    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2[0], "ETHUSD", SELL, 1, 1);
}

TEST_F(UnitInvalidOrders, SimpleManyInvalidOrder)
{
    using nutc::testing_utils::add_client_simple;
    using nutc::testing_utils::modify_capital_simple;
    using nutc::testing_utils::modify_holdings_simple;

    add_client_simple(manager_, "A");
    add_client_simple(manager_, "B");
    add_client_simple(manager_, "C");
    add_client_simple(manager_, "D");

    modify_capital_simple(manager_, "B", -STARTING_CAPITAL);
    modify_holdings_simple(manager_, "A", "ETHUSD", DEFAULT_QUANTITY);
    modify_holdings_simple(manager_, "B", "ETHUSD", DEFAULT_QUANTITY);
    modify_holdings_simple(manager_, "C", "ETHUSD", DEFAULT_QUANTITY);
    modify_holdings_simple(manager_, "D", "ETHUSD", DEFAULT_QUANTITY);

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
    ASSERT_EQ(updates2.size(), 0);
    ASSERT_EQ(matches3.size(), 0);
    ASSERT_EQ(updates3.size(), 1);

    // Should match two orders and throw out the invalid order (2)
    auto [matches4, updates4] = add_to_engine_(order4);
    ASSERT_EQ(matches4.size(), 2);
    ASSERT_EQ(updates4.size(), 3);

    ASSERT_EQ_MATCH(matches4[0], "ETHUSD", "A", "D", SELL, 1, 1);
    ASSERT_EQ_MATCH(matches4[1], "ETHUSD", "C", "D", SELL, 1, 1);

    ASSERT_EQ_OB_UPDATE(updates4[0], "ETHUSD", BUY, 1, 0);
    ASSERT_EQ_OB_UPDATE(updates4[1], "ETHUSD", BUY, 1, 0);
    ASSERT_EQ_OB_UPDATE(updates4[2], "ETHUSD", SELL, 1, 1);
}
