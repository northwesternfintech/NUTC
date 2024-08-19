#include "exchange/bots/bot_types/market_maker.hpp"
#include "exchange/bots/shared_bot_state.hpp"
#include "shared/types/decimal_price.hpp"
#include "shared/types/ticker.hpp"
#include "util/helpers/test_random_generator.hpp"

#include <gtest/gtest.h>

using namespace nutc;

class UnitMarketMakerBotTest : public ::testing::Test {
protected:
    util::decimal_price interest_limit{10000.0};
    util::Ticker ticker{"ABC"};
    bots::MarketMakerBot<test::ReturnMeanNoiseGenerator> bot{ticker, interest_limit};
};

TEST_F(UnitMarketMakerBotTest, PlacesExpectedOrdersWithNoLeanOrSpread)
{
    util::decimal_price theo = 5.0;
    bots::shared_bot_state state{{}, theo, {}, {}, {}};
    bot.take_action(state);

    ASSERT_EQ(bot.read_orders().size(), 6);
}

// TODO: more extensive

