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

    theo_ = bot_container.get_theo();

    auto calculate_pnl = [&engine_ref](const bots::GenericBot& bot) {
        double capital = bot.get_capital();
        double held_stock = bot.get_held_stock();
        double stock_value = engine_ref.get_midprice() * held_stock;
        return capital + stock_value;
    };

    auto update_bot_state = [&calculate_pnl](auto&& container, BotStates& state) {
        state.num_bots_ = container.size();
        for (const auto& [id, bot] : container) {
            state.min_open_bids_ = std::min(state.min_open_bids_, bot.get_open_bids());
            state.min_open_asks_ = std::min(state.min_open_asks_, bot.get_open_asks());
            state.max_open_bids_ = std::max(state.max_open_bids_, bot.get_open_bids());
            state.max_open_asks_ = std::max(state.max_open_asks_, bot.get_open_asks());
            state.min_utilization_ =
                std::min(state.min_utilization_, bot.get_capital_utilization());
            state.max_utilization_ =
                std::max(state.max_utilization_, bot.get_capital_utilization());
            state.min_pnl_ = std::min(state.min_pnl_, calculate_pnl(bot));
            state.max_pnl_ = std::max(state.max_pnl_, calculate_pnl(bot));

            state.avg_open_bids_ += static_cast<double>(bot.get_open_bids());
            state.avg_open_asks_ += static_cast<double>(bot.get_open_asks());
            state.avg_bid_interest_ += bot.get_long_interest();
            state.avg_ask_interest_ += bot.get_short_interest();
            state.avg_utilization_ += bot.get_capital_utilization();
            state.avg_pnl_ += calculate_pnl(bot);
        }
        state.avg_open_bids_ /= static_cast<double>(state.num_bots_);
        state.avg_open_asks_ /= static_cast<double>(state.num_bots_);
        state.avg_bid_interest_ /= static_cast<double>(state.num_bots_);
        state.avg_ask_interest_ /= static_cast<double>(state.num_bots_);
        state.avg_utilization_ /= static_cast<double>(state.num_bots_);
        state.avg_pnl_ /= static_cast<double>(state.num_bots_);
    };
    update_bot_state(bot_container.get_market_makers(), mm_state_);
    update_bot_state(bot_container.get_retail_traders(), retail_state_);
}

} // namespace dashboard

} // namespace nutc
