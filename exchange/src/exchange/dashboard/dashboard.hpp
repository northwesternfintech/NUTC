#pragma once

#include "exchange/dashboard/state/ticker_state.hpp"
#include "exchange/tick_manager/tick_observer.hpp"

#include <ncurses.h>

#include <deque>
#include <fstream>

namespace nutc {
namespace dashboard {

// NOLINTBEGIN

class Dashboard : public ticks::TickObserver {
public:
    void
    on_tick(uint64_t tick) override
    {
        mainLoop(tick);
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

private:
    void static drawTickerLayout(WINDOW* window, int start_y, size_t num_tickers);
    void static displayStockTickerData(
        WINDOW* window, int start_y, int start_x, const TickerState& ticker
    );
    void static displayStockTickers(WINDOW* window, int start_y);
    void displayLog(WINDOW* window, int start_y);
    void static displayLeaderboard(WINDOW* window, int start_y);
    void static displayPerformance(WINDOW* window, int start_y);

    void static calculate_ticker_metrics();

    void mainLoop(uint64_t tick);

    static void* read_pipe_and_log(void* args);

    std::deque<std::string> log_queue_{};

    FILE* err_file_;

    WINDOW* ticker_window_;
    WINDOW* log_window_;
    WINDOW* leaderboard_window_;
    WINDOW* performance_window_;

    std::streampos log_pos_ = std::ios::beg;

    char current_window_ = '1';

    std::ifstream log_file_;

    Dashboard();
    ~Dashboard() = default;
};

} // namespace dashboard
} // namespace nutc

//
//
// NOLINTEND
