#include "ticker_state.hpp"

#include "exchange/tickers/manager/ticker_manager.hpp"

#include <algorithm>

namespace nutc {
namespace dashboard {

void
TickerState::on_tick(uint64_t)
{
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

        // mm_open_bids_.push_back(bot.get_open_bids());
        // mm_open_asks_.push_back(bot.get_open_asks());
        // mm_bid_interest_.push_back(bot.get_long_interest());
        // mm_ask_interest_.push_back(bot.get_short_interest());
        // mm_utilization_.push_back(bot.get_utilization());
    }
    mm_avg_open_bids_ /= num_mm_bots_;
    mm_avg_open_asks_ /= num_mm_bots_;
    mm_avg_bid_interest_ /= static_cast<float>(num_mm_bots_);
    mm_avg_ask_interest_ /= static_cast<float>(num_mm_bots_);
    mm_avg_utilization_ /= static_cast<float>(num_mm_bots_);
}

} // namespace dashboard
} // namespace nutc
