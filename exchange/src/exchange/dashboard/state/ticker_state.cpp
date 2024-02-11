#include "ticker_state.hpp"

#include "exchange/bots/bot_types/generic_bot.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"

#include <algorithm>

namespace nutc {
namespace dashboard {

void
TickerState::on_tick(uint64_t tick)
{
    if (tick % 4 != 0)
        return;
    reset_();
    bots::BotContainer& bot_container =
        engine_manager::EngineManager::get_instance().get_bot_container(TICKER);

    std::optional<engine_ref_t> engine =
        engine_manager::EngineManager::get_instance().get_engine(TICKER);
    assert(engine.has_value());

    nutc::matching::Engine& engine_ref = engine.value().get();
    midprice_ = engine_ref.get_midprice();
    spread_ = engine_ref.get_spread();
    auto [asks, bids] = engine_ref.get_spread_nums();
    num_asks_ = asks;
    num_bids_ = bids;

    theo_ = static_cast<float>(bot_container.get_theo());
    num_mm_bots_ = bot_container.get_market_makers().size();
    assert(num_mm_bots_ > 0);

    auto calculate_pnl = [&engine_ref](const bots::GenericBot& bot) {
      float capital = bot.get_capital();
      float held_stock = bot.get_held_stock();
      float stock_value = engine_ref.get_midprice() * held_stock;
      return capital + stock_value;
  };

    for (const auto& [id, bot] : bot_container.get_market_makers()) {
        mm_min_open_bids_ = std::min(mm_min_open_bids_, bot.get_open_bids());
        mm_min_open_asks_ = std::min(mm_min_open_asks_, bot.get_open_asks());
        mm_max_open_bids_ = std::max(mm_max_open_bids_, bot.get_open_bids());
        mm_max_open_asks_ = std::max(mm_max_open_asks_, bot.get_open_asks());
        mm_min_utilization_ =
            std::min(mm_min_utilization_, bot.get_capital_utilization());
        mm_max_utilization_ =
            std::max(mm_max_utilization_, bot.get_capital_utilization());

        mm_avg_open_bids_ += bot.get_open_bids();
        mm_avg_open_asks_ += bot.get_open_asks();
        mm_avg_bid_interest_ += bot.get_long_interest();
        mm_avg_ask_interest_ += bot.get_short_interest();
        mm_avg_utilization_ += bot.get_capital_utilization();
    }
    mm_avg_open_bids_ /= num_mm_bots_;
    mm_avg_open_asks_ /= num_mm_bots_;
    mm_avg_bid_interest_ /= static_cast<float>(num_mm_bots_);
    mm_avg_ask_interest_ /= static_cast<float>(num_mm_bots_);
    mm_avg_utilization_ /= static_cast<float>(num_mm_bots_);

    num_retail_bots_ = bot_container.get_retail_traders().size();
    assert(num_retail_bots_ > 0);
    for (const auto& [id, bot] : bot_container.get_retail_traders()) {
        retail_min_open_bids_ = std::min(retail_min_open_bids_, bot.get_open_bids());
        retail_min_open_asks_ = std::min(retail_min_open_asks_, bot.get_open_asks());
        retail_max_open_bids_ = std::max(retail_max_open_bids_, bot.get_open_bids());
        retail_max_open_asks_ = std::max(retail_max_open_asks_, bot.get_open_asks());
        retail_min_utilization_ =
            std::min(retail_min_utilization_, bot.get_capital_utilization());
        retail_max_utilization_ =
            std::max(retail_max_utilization_, bot.get_capital_utilization());
        retail_min_pnl_ = std::min(retail_min_pnl_, calculate_pnl(bot));
        retail_max_pnl_ = std::max(retail_max_pnl_, calculate_pnl(bot));

        retail_avg_open_bids_ += bot.get_open_bids();
        retail_avg_open_asks_ += bot.get_open_asks();
        retail_avg_bid_interest_ += bot.get_long_interest();
        retail_avg_ask_interest_ += bot.get_short_interest();
        retail_avg_utilization_ += bot.get_capital_utilization();
        retail_avg_pnl_ += calculate_pnl(bot);
    }
    retail_avg_open_bids_ /= num_retail_bots_;
    retail_avg_open_asks_ /= num_retail_bots_;
    retail_avg_bid_interest_ /= static_cast<float>(num_retail_bots_);
    retail_avg_ask_interest_ /= static_cast<float>(num_retail_bots_);
    retail_avg_utilization_ /= static_cast<float>(num_retail_bots_);
    retail_avg_pnl_ /= static_cast<float>(num_retail_bots_);
}

} // namespace dashboard
} // namespace nutc
