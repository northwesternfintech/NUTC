#pragma once

#include "exchange/tick_scheduler/tick_observer.hpp"

#ifndef DASHBOARD
#  include "exchange/tickers/manager/ticker_manager.hpp"
#  include "exchange/traders/trader_container.hpp"
#  include "shared/config/config_loader.hpp"

#  include <prometheus/exposer.h>
#  include <prometheus/gauge.h>
#  include <prometheus/registry.h>

namespace nutc {
namespace dashboard {
namespace ps = prometheus;

// THIS IS AN EMPTY VERSION OF DASHBOARD FOR WHEN WE DISABLE IT
// IF YOU WANT TO MODIFY THE ACTUAL DASHBOARD, SCROLL DOWN

class Dashboard : public ticks::TickObserver {
public:
    void
    on_tick(uint64_t) override
    {
        static ps::Exposer exposer{"0.0.0.0:4152"};
        static auto registry = std::make_shared<ps::Registry>();
        static auto& pnl_gauge = ps::BuildGauge().Name("pnl").Register(*registry);
        static auto& capital_gauge =
            ps::BuildGauge().Name("capital").Register(*registry);
        static auto& portfolio_gauge =
            ps::BuildGauge().Name("portfolio_value").Register(*registry);
        static bool have_reg = false;
        if (!have_reg) {
            have_reg = true;
            exposer.RegisterCollectable(registry);
        }
        const auto& tickers = config::Config::get().get_tickers();

        auto portfolio_value = [&](const auto& trader) {
            double pnl = 0.0;
            for (const auto& ticker_conf : tickers) {
                double amount_held = trader->get_holdings(ticker_conf.TICKER);
                double midprice =
                    engine_manager::EngineManager::get_instance().get_midprice(
                        ticker_conf.TICKER
                    );
                pnl += amount_held * midprice;
            }
            return pnl;
        };

        traders::TraderContainer& client_manager =
            traders::TraderContainer::get_instance();
        for (const auto& trader_p : client_manager.get_traders()) {
            auto trader = trader_p.second;

            double capital = trader->get_capital();
            double portfolio = portfolio_value(trader);
            double pnl = capital + portfolio - trader->get_initial_capital();
            auto name = trader->get_id();
            pnl_gauge
                .Add({
                    {"name", name}
            })
                .Set(pnl);
            capital_gauge
                .Add({
                    {"name", name}
            })
                .Set(capital);
            portfolio_gauge
                .Add({
                    {"name", name}
            })
                .Set(portfolio);
        }
    }

    static Dashboard&
    get_instance()
    {
        static Dashboard instance;
        return instance;
    }

    Dashboard& operator=(const Dashboard&) = delete;
    Dashboard(const Dashboard&) = delete;
    Dashboard& operator=(Dashboard&&) = delete;
    Dashboard(Dashboard&&) = delete;

    void
    close()
    {}

    ~Dashboard() override = default;

private:
    Dashboard() = default;
};
} // namespace dashboard
} // namespace nutc
#else
#  include "exchange/dashboard/state/ticker_state.hpp"

#  include <ncurses.h>
#  undef timeout
#  undef OK
#  include <deque>
#  include <fstream>

namespace nutc {
namespace dashboard {
/**
 * @brief Displays the dashboard and all respective metrics. Messy, but that's okay
 * because it's very isolated Performance not a big consideration
 */
class Dashboard : public ticks::TickObserver {
    std::deque<std::string> log_queue_{};

    FILE* err_file_;

    WINDOW* ticker_window_;
    WINDOW* log_window_;
    WINDOW* leaderboard_window_;
    WINDOW* performance_window_;

    std::streampos log_pos_ = std::ios::beg;

    char current_window_ = '1';

    std::ifstream log_file_;
    const uint16_t TICK_HZ;
    const uint8_t DISPLAY_HZ;

public:
    void
    on_tick(uint64_t tick) override
    {
        main_loop_(tick);
    }

    static Dashboard&
    get_instance()
    {
        static Dashboard instance;
        return instance;
    }

    Dashboard& operator=(const Dashboard&) = delete;
    Dashboard(const Dashboard&) = delete;
    Dashboard& operator=(Dashboard&&) = delete;
    Dashboard(Dashboard&&) = delete;
    void close();

    ~Dashboard() override { close(); }

private:
    void static draw_ticker_layout(WINDOW* window, int start_y, size_t num_tickers);
    void static display_stock_ticker_data(
        WINDOW* window, int start_y, int start_x, const TickerState& ticker
    );
    void static display_stock_tickers(WINDOW* window, int start_y);
    void display_log_(WINDOW* window, int start_y);
    void static display_leaderboard(WINDOW* window, int start_y);
    void static display_performance(WINDOW* window, int start_y);

    void static calculate_ticker_metrics();

    void main_loop_(uint64_t tick);

    static void* read_pipe_and_log(void* args);

    Dashboard();
}; // namespace dashboard

} // namespace dashboard
} // namespace nutc
#endif

//
//
