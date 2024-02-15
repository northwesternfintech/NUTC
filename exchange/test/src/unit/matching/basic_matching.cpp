#include "exchange/tickers/engine/engine.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::messages::SIDE::BUY;
using nutc::messages::SIDE::SELL;

class UnitBasicMatching : public ::testing::Test {
protected:
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        manager_.add_local_trader("ABC");
        manager_.add_local_trader("DEF");

        manager_.get_trader("ABC")->modify_capital(STARTING_CAPITAL);
        manager_.get_trader("DEF")->modify_capital(STARTING_CAPITAL);

        manager_.get_trader("ABC")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("DEF")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    }

    void
    TearDown() override
    {
        manager_.reset();
        SetUp();
    }

    ClientManager& manager_ = nutc::manager::ClientManager::get_instance(); // NOLINT(*)
    Engine engine_{}; // NOLINT (*)

    nutc::matching::match_result_t
    add_to_engine_(MarketOrder order)
    {
        return engine_.match_order(std::move(order), manager_); // NOLINT(*)
    }
};

TEST_F(UnitBasicMatching, SimpleAddRemove)
{
    double capital_1 = manager_.get_trader("ABC")->get_capital();
    manager_.get_trader("ABC")->modify_capital(100);
    double capital_2 = manager_.get_trader("ABC")->get_capital();
    ASSERT_EQ(capital_1 + 100, capital_2);
}

TEST_F(UnitBasicMatching, SimpleMatch)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", BUY, 1, 1);

    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 1);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", BUY, 1, 0);
    ASSERT_EQ_MATCH(matches2.at(0), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
}

TEST_F(UnitBasicMatching, CorrectBuyPricingOrder)
{
    MarketOrder buy1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder buy2{"ABC", BUY, "ETHUSD", 1, 2};
    MarketOrder buy3{"ABC", BUY, "ETHUSD", 1, 3};
    MarketOrder buy4{"ABC", BUY, "ETHUSD", 1, 4};
    MarketOrder sell1{"DEF", SELL, "ETHUSD", 1, 1};

    // Place cheapest buy orders first, then most expensive
    auto [matches1, ob_updates1] = add_to_engine_(buy1);
    auto [matches3, ob_updates3] = add_to_engine_(buy3);
    auto [matches2, ob_updates2] = add_to_engine_(buy2);
    auto [matches4, ob_updates4] = add_to_engine_(buy4);
    ASSERT_EQ(ob_updates1.size(), 1);
    ASSERT_EQ(matches1.size(), 0);
    ASSERT_EQ_OB_UPDATE(ob_updates1.at(0), "ETHUSD", BUY, 1, 1);
    ASSERT_EQ(ob_updates3.size(), 1);
    ASSERT_EQ(matches3.size(), 0);
    ASSERT_EQ_OB_UPDATE(ob_updates3.at(0), "ETHUSD", BUY, 3, 1);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", BUY, 2, 1);
    ASSERT_EQ(ob_updates4.size(), 1);
    ASSERT_EQ(matches4.size(), 0);
    ASSERT_EQ_OB_UPDATE(ob_updates4.at(0), "ETHUSD", BUY, 4, 1);

    auto [matches5, ob_updates5] = add_to_engine_(sell1);
    ASSERT_EQ(ob_updates5.size(), 1);
    ASSERT_EQ(matches5.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates5.at(0), "ETHUSD", BUY, 4, 0);

    auto [matches6, ob_updates6] = add_to_engine_(sell1);
    ASSERT_EQ(ob_updates6.size(), 1);
    ASSERT_EQ(matches6.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates6.at(0), "ETHUSD", BUY, 3, 0);
}

TEST_F(UnitBasicMatching, NoMatchThenMatchBuy)
{
    MarketOrder order1{"ABC", SELL, "ETHUSD", 1, DEFAULT_QUANTITY};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", BUY, "ETHUSD", 1, 2};
    auto [matches, ob_updates] = add_to_engine_(order1);
    auto [matches2, ob_updates2] = add_to_engine_(order2);
    auto [matches3, ob_updates3] = add_to_engine_(order3);
    ASSERT_EQ(ob_updates.size(), 1);
    ASSERT_EQ(ob_updates2.size(), 1);
    // ASSERT_EQ(ob_updates3.size(), 1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(matches3.size(), 1);
}

TEST_F(UnitBasicMatching, NoMatchThenMatchSell)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", BUY, "ETHUSD", 1, DEFAULT_QUANTITY};

    constexpr int HALF_QUANTITY = DEFAULT_QUANTITY / 2;

    MarketOrder order3{"DEF", SELL, "ETHUSD", 1, HALF_QUANTITY};
    auto [matches, ob_updates] = add_to_engine_(order1);
    auto [matches2, ob_updates2] = add_to_engine_(order2);
    auto [matches3, ob_updates3] = add_to_engine_(order1);
    auto [matches4, ob_updates4] = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(matches3.size(), 0);
    ASSERT_EQ(matches4.size(), 1);
}

TEST_F(UnitBasicMatching, PassivePriceMatch)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 2};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", BUY, 2, 1);

    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 1);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", BUY, 2, 0);
    ASSERT_EQ_MATCH(matches2.at(0), "ETHUSD", "ABC", "DEF", SELL, 2, 1);
}

TEST_F(UnitBasicMatching, PartialFill)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 2, 1};
    MarketOrder order2{"DEF", SELL, "ETHUSD", 1, 1};
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", BUY, 1, 2);

    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 1);
    ASSERT_EQ(ob_updates2.size(), 2);
    ASSERT_EQ_MATCH(matches2.at(0), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", BUY, 1, 0);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(1), "ETHUSD", BUY, 1, 1);
}

TEST_F(UnitBasicMatching, MultipleFill)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", SELL, "ETHUSD", 2, 1};
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", BUY, 1, 1);

    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", BUY, 1, 1);

    auto [matches3, ob_updates3] = add_to_engine_(order3);
    ASSERT_EQ(matches3.size(), 2);
    ASSERT_EQ(ob_updates3.size(), 2);
    ASSERT_EQ_MATCH(matches3.at(0), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    ASSERT_EQ_MATCH(matches3.at(1), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    ASSERT_EQ_OB_UPDATE(ob_updates3.at(0), "ETHUSD", BUY, 1, 0);
    ASSERT_EQ_OB_UPDATE(ob_updates3.at(1), "ETHUSD", BUY, 1, 0);
}

TEST_F(UnitBasicMatching, MultiplePartialFill)
{
    MarketOrder order1{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", BUY, "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", SELL, "ETHUSD", 3, 1};
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", BUY, 1, 1);

    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", BUY, 1, 1);

    auto [matches3, ob_updates3] = add_to_engine_(order3);
    ASSERT_EQ(matches3.size(), 2);
    ASSERT_EQ(ob_updates3.size(), 3);
    ASSERT_EQ_MATCH(matches3.at(0), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    ASSERT_EQ_MATCH(matches3.at(1), "ETHUSD", "ABC", "DEF", SELL, 1, 1);
    ASSERT_EQ_OB_UPDATE(ob_updates3.at(0), "ETHUSD", BUY, 1, 0);
    ASSERT_EQ_OB_UPDATE(ob_updates3.at(1), "ETHUSD", BUY, 1, 0);
    ASSERT_EQ_OB_UPDATE(ob_updates3.at(2), "ETHUSD", SELL, 1, 1);
}

TEST_F(UnitBasicMatching, SimpleMatchReversed)
{
    MarketOrder order1{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", BUY, "ETHUSD", 1, 1};
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", SELL, 1, 1);
    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 1);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", SELL, 1, 0);
    ASSERT_EQ_MATCH(matches2.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
}

TEST_F(UnitBasicMatching, PassivePriceMatchReversed)
{
    MarketOrder order1{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order2{"DEF", BUY, "ETHUSD", 1, 2};
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", SELL, 1, 1);

    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 1);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ(matches2.at(0).price, 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", SELL, 1, 0);
    ASSERT_EQ_MATCH(matches2.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
}

TEST_F(UnitBasicMatching, PartialFillReversed)
{
    MarketOrder order1{"ABC", SELL, "ETHUSD", 2, 1};
    MarketOrder order2{"DEF", BUY, "ETHUSD", 1, 1};
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", SELL, 1, 2);
    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 1);
    ASSERT_EQ(ob_updates2.size(), 2);
    ASSERT_EQ_MATCH(matches2.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", SELL, 1, 0);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(1), "ETHUSD", SELL, 1, 1);
}

TEST_F(UnitBasicMatching, MultipleFillReversed)
{
    MarketOrder order1{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", BUY, "ETHUSD", 2, 1};
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", SELL, 1, 1);

    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", SELL, 1, 1);

    auto [matches3, ob_updates3] = add_to_engine_(order3);
    ASSERT_EQ(matches3.size(), 2);
    ASSERT_EQ(ob_updates3.size(), 2);
    ASSERT_EQ_MATCH(matches3.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
    ASSERT_EQ_MATCH(matches3.at(1), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
    ASSERT_EQ_OB_UPDATE(ob_updates3.at(0), "ETHUSD", SELL, 1, 0);
    ASSERT_EQ_OB_UPDATE(ob_updates3.at(1), "ETHUSD", SELL, 1, 0);
}

TEST_F(UnitBasicMatching, MultiplePartialFillReversed)
{
    MarketOrder order1{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order2{"ABC", SELL, "ETHUSD", 1, 1};
    MarketOrder order3{"DEF", BUY, "ETHUSD", 3, 1};
    auto [matches, ob_updates] = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    ASSERT_EQ(ob_updates.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates.at(0), "ETHUSD", SELL, 1, 1);

    auto [matches2, ob_updates2] = add_to_engine_(order2);
    ASSERT_EQ(matches2.size(), 0);
    ASSERT_EQ(ob_updates2.size(), 1);
    ASSERT_EQ_OB_UPDATE(ob_updates2.at(0), "ETHUSD", SELL, 1, 1);

    auto [matches3, ob_updates3] = add_to_engine_(order3);
    ASSERT_EQ(matches3.size(), 2);
    ASSERT_EQ(ob_updates3.size(), 3);
    ASSERT_EQ_MATCH(matches3.at(0), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
    ASSERT_EQ_MATCH(matches3.at(1), "ETHUSD", "DEF", "ABC", BUY, 1, 1);
    ASSERT_EQ_OB_UPDATE(ob_updates3.at(0), "ETHUSD", SELL, 1, 0);
    ASSERT_EQ_OB_UPDATE(ob_updates3.at(1), "ETHUSD", SELL, 1, 0);
    ASSERT_EQ_OB_UPDATE(ob_updates3.at(2), "ETHUSD", BUY, 1, 1);
}
