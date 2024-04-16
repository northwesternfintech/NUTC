#pragma once

#include "exchange/tick_scheduler/tick_observer.hpp"

#ifndef DASHBOARD
namespace nutc {
namespace dashboard {
// THIS IS AN EMPTY VERSION OF DASHBOARD FOR WHEN WE DISABLE IT
// IF YOU WANT TO MODIFY THE ACTUAL DASHBOARD, SCROLL DOWN

class Dashboard : public ticks::TickObserver {
public:
    constexpr void
    on_tick(uint64_t) override
    {}

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
    ~Dashboard() override = default;

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
};

} // namespace dashboard
} // namespace nutc
#endif

//
//
