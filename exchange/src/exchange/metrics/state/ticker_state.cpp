#include "ticker_state.hpp"

#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"

#include <algorithm>

namespace nutc {
namespace dashboard {

void
TickerState::calculate_metrics()
{
    reset_();
    bots::BotContainer& bot_container =
        engine_manager::EngineManager::get_instance().get_bot_container(TICKER);

    auto& engine_ref = engine_manager::EngineManager::get_instance();
    const auto& order_container =
        engine_ref.get_engine(TICKER).engine.get_order_container();

    midprice_ = engine_ref.get_midprice(TICKER);
    spread_ = order_container.get_spread();
    auto [asks, bids] = order_container.get_spread_nums();
    num_asks_ = asks;
    num_bids_ = bids;

    theo_ = bot_container.get_theo();
    uint64_t new_num_matches =
        engine_manager::EngineManager::get_instance().get_num_matches(TICKER);
    matches_since_last_tick_ = new_num_matches - num_matches_;
    num_matches_ = new_num_matches;

    auto update_bot_state = [](auto&& container, BotStates& state) {
        state.num_bots_ = container.size();
        for (const auto& bot : container) {
            state.total_capital_held_ +=
                bot->get_interest_limit() + bot->get_capital_delta();
            state.num_bots_active_++;
            state.min_open_bids_ = std::min(state.min_open_bids_, bot->get_open_bids());
            state.min_open_asks_ = std::min(state.min_open_asks_, bot->get_open_asks());
            state.max_open_bids_ = std::max(state.max_open_bids_, bot->get_open_bids());
            state.max_open_asks_ = std::max(state.max_open_asks_, bot->get_open_asks());
            state.min_utilization_ =
                std::min(state.min_utilization_, bot->get_capital_utilization());
            state.max_utilization_ =
                std::max(state.max_utilization_, bot->get_capital_utilization());
            state.min_pnl_ = std::min(state.min_pnl_, 0.0);
            state.max_pnl_ = std::max(state.max_pnl_, 0.0);

            state.avg_open_bids_ += static_cast<double>(bot->get_open_bids());
            state.avg_open_asks_ += static_cast<double>(bot->get_open_asks());
            state.avg_bid_interest_ += bot->get_long_interest();
            state.avg_ask_interest_ += bot->get_short_interest();
            state.avg_utilization_ += bot->get_capital_utilization();
        }
        if (state.num_bots_active_ == 0) {
            return;
        }
        state.avg_open_bids_ /= static_cast<double>(state.num_bots_active_);
        state.avg_open_asks_ /= static_cast<double>(state.num_bots_active_);
        state.avg_bid_interest_ /= static_cast<double>(state.num_bots_active_);
        state.avg_ask_interest_ /= static_cast<double>(state.num_bots_active_);
        state.avg_utilization_ /= static_cast<double>(state.num_bots_active_);
        state.avg_pnl_ /= static_cast<double>(state.num_bots_active_);
        state.percent_active_ = static_cast<double>(state.num_bots_active_)
                                / static_cast<double>(state.num_bots_);
    };
    update_bot_state(bot_container.get_bots(), mm_state_);
}

} // namespace dashboard

} // namespace nutc
