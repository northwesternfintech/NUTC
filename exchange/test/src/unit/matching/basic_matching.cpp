#include "config.h"
#include "exchange/traders/trader_container.hpp"
#include "exchange/traders/trader_types/local_trader.hpp"
#include "test_utils/macros.hpp"

#include <gtest/gtest.h>

using nutc::util::Side::buy;
using nutc::util::Side::sell;

class UnitBasicMatching : public ::testing::Test {
protected:
    using LocalTrader = nutc::traders::LocalTrader;
    static constexpr const int DEFAULT_QUANTITY = 1000;

    void
    SetUp() override
    {
        manager_.add_trader<LocalTrader>("ABC", TEST_STARTING_CAPITAL);
        manager_.add_trader<LocalTrader>("DEF", TEST_STARTING_CAPITAL);

        manager_.get_trader("ABC")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
        manager_.get_trader("DEF")->modify_holdings("ETHUSD", DEFAULT_QUANTITY);
    }

    void
    TearDown() override
    {
        manager_.reset();
        SetUp();
    }

    TraderContainer& manager_ =
        nutc::traders::TraderContainer::get_instance(); // NOLINT(*)
    Engine engine_{TEST_ORDER_EXPIRATION_TICKS};        // NOLINT (*)

    std::vector<nutc::matching::stored_match>
    add_to_engine_(const stored_order& order)
    {
        return engine_.match_order(order);
    }
};

TEST_F(UnitBasicMatching, SimpleMatch)
{
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("DEF"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "ABC", "DEF", nutc::util::Side::sell, 1, 1);
}

TEST_F(UnitBasicMatching, CorrectBuyPricingOrder)
{
    stored_order buy1{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order buy2{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 2, 0
    };
    stored_order buy3{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 3, 0
    };
    stored_order buy4{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 4, 0
    };
    stored_order sell1{
        manager_.get_trader("DEF"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };

    // Place cheapest buy orders first, then most expensive
    auto matches = add_to_engine_(buy1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(buy3);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(buy2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(buy4);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(sell1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "ABC", "DEF", nutc::util::Side::sell, 4, 1);
    matches = add_to_engine_(sell1);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "ABC", "DEF", nutc::util::Side::sell, 3, 1);
}

TEST_F(UnitBasicMatching, NoMatchThenMatchBuy)
{
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("DEF"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    stored_order order3{
        manager_.get_trader("DEF"), nutc::util::Side::buy, "ETHUSD", 1, 2, 0
    };
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "DEF", "ABC", nutc::util::Side::buy, 1, 1);
}

TEST_F(UnitBasicMatching, NoMatchThenMatchSell)
{
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("DEF"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order3{
        manager_.get_trader("DEF"), nutc::util::Side::sell, "ETHUSD", 2, 0, 0
    };

    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "ABC", "DEF", nutc::util::Side::sell, 1, 1);
    ASSERT_EQ_MATCH(matches[1], "ETHUSD", "DEF", "DEF", nutc::util::Side::sell, 1, 1);
}

TEST_F(UnitBasicMatching, PassivePriceMatch)
{
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 2, 0
    };
    stored_order order2{
        manager_.get_trader("DEF"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches[0], "ETHUSD", "ABC", "DEF", nutc::util::Side::sell, 2, 1);
}

TEST_F(UnitBasicMatching, PartialFill)
{
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 2, 1, 0
    };
    stored_order order2{
        manager_.get_trader("DEF"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(
        matches.at(0), "ETHUSD", "ABC", "DEF", nutc::util::Side::sell, 1, 1
    );
}

TEST_F(UnitBasicMatching, MultipleFill)
{
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order3{
        manager_.get_trader("DEF"), nutc::util::Side::sell, "ETHUSD", 2, 1, 0
    };
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(
        matches.at(0), "ETHUSD", "ABC", "DEF", nutc::util::Side::sell, 1, 1
    );
    ASSERT_EQ_MATCH(
        matches.at(1), "ETHUSD", "ABC", "DEF", nutc::util::Side::sell, 1, 1
    );
}

TEST_F(UnitBasicMatching, MultiplePartialFill)
{
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("ABC"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    stored_order order3{
        manager_.get_trader("DEF"), nutc::util::Side::sell, "ETHUSD", 3, 1, 0
    };
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(
        matches.at(0), "ETHUSD", "ABC", "DEF", nutc::util::Side::sell, 1, 1
    );
    ASSERT_EQ_MATCH(
        matches.at(1), "ETHUSD", "ABC", "DEF", nutc::util::Side::sell, 1, 1
    );
}

TEST_F(UnitBasicMatching, SimpleMatchReversed)
{
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("DEF"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "DEF", "ABC", nutc::util::Side::buy, 1, 1);
}

TEST_F(UnitBasicMatching, PassivePriceMatchReversed)
{
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("DEF"), nutc::util::Side::buy, "ETHUSD", 1, 2, 0
    };
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ(matches.at(0).price, 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "DEF", "ABC", nutc::util::Side::buy, 1, 1);
}

TEST_F(UnitBasicMatching, PartialFillReversed)
{
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::sell, "ETHUSD", 2, 1, 0
    };
    stored_order order2{
        manager_.get_trader("DEF"), nutc::util::Side::buy, "ETHUSD", 1, 1, 0
    };
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);
    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "DEF", "ABC", nutc::util::Side::buy, 1, 1);
}

TEST_F(UnitBasicMatching, MultipleFillReversed)
{
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("ABC"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    stored_order order3{
        manager_.get_trader("DEF"), nutc::util::Side::buy, "ETHUSD", 2, 1, 0
    };
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "DEF", "ABC", nutc::util::Side::buy, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETHUSD", "DEF", "ABC", nutc::util::Side::buy, 1, 1);
}

TEST_F(UnitBasicMatching, MultiplePartialFillReversed)
{
    stored_order order1{
        manager_.get_trader("ABC"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    stored_order order2{
        manager_.get_trader("ABC"), nutc::util::Side::sell, "ETHUSD", 1, 1, 0
    };
    stored_order order3{
        manager_.get_trader("DEF"), nutc::util::Side::buy, "ETHUSD", 3, 1, 0
    };
    auto matches = add_to_engine_(order1);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order2);
    ASSERT_EQ(matches.size(), 0);

    matches = add_to_engine_(order3);
    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ_MATCH(matches.at(0), "ETHUSD", "DEF", "ABC", nutc::util::Side::buy, 1, 1);
    ASSERT_EQ_MATCH(matches.at(1), "ETHUSD", "DEF", "ABC", nutc::util::Side::buy, 1, 1);
}
