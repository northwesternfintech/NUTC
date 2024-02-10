#include "dashboard.hpp"

#include "state/global_metrics.hpp"

#include <ncurses.h>

#include <cassert>

namespace nutc {
namespace dashboard {

void
Dashboard::init()
{
  err_file_ = freopen("logs/error_log.txt", "w", stderr);
    initscr();
    noecho();
    curs_set(0);
    cbreak();

    mainLoop();
}

void
Dashboard::close()
{
  fflush(err_file_);
  fclose(err_file_);
    endwin();
}

void
Dashboard::drawTickerLayout(size_t num_tickers)
{
    assert(num_tickers <= 4);

    int x_max{};
    int y_max{};
    getmaxyx(stdscr, y_max, x_max);

    for (size_t i = 1; i < num_tickers + 1; ++i) {
        for (int j = 0; j < y_max; ++j) {
            mvprintw(j, i * x_max / num_tickers, "|"); // NOLINT
        }
    }

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_CYAN, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
        init_pair(4, COLOR_BLUE, COLOR_BLACK);
        init_pair(5, COLOR_RED, COLOR_BLACK);
    }

    attron(COLOR_PAIR(1));
    mvprintw(0, x_max / 2 - 8, "Stock Ticker Pane");
    attroff(COLOR_PAIR(1));
}

void
Dashboard::displayStockTickerData(int start_y, int start_x, const TickerState& ticker)
{
    mvprintw(start_y, start_x, "Ticker: %s", ticker.TICKER.c_str());
    mvprintw(
        start_y + 2, start_x, "Midprice: %.2f", static_cast<double>(ticker.midprice_)
    );
    mvprintw(
        start_y + 3, start_x, "Theo Price: %.2f", static_cast<double>(ticker.theo_)
    );
    mvprintw(
        start_y + 4, start_x, "Spread: %.2f %.2f",
        static_cast<double>(ticker.spread_.first),
        static_cast<double>(ticker.spread_.second)
    );
    mvprintw(
        start_y + 5, start_x, "Bids/asks: %lu %lu", ticker.num_bids_, ticker.num_asks_
    );
    mvprintw(start_y + 7, start_x, "MM bots: %lu", ticker.num_mm_bots_);
    mvprintw(
        start_y + 8, start_x, "MM min/max/avg num bids: %lu %lu %lu",
        ticker.mm_min_open_bids_, ticker.mm_max_open_bids_, ticker.mm_avg_open_bids_
    );
    mvprintw(
        start_y + 9, start_x, "MM min/max/avg num asks: %lu %lu %lu",
        ticker.mm_min_open_asks_, ticker.mm_max_open_asks_, ticker.mm_avg_open_asks_
    );
    mvprintw(
        start_y + 10, start_x, "MM min/max/avg utilization: %f %f %f",
        ticker.mm_min_utilization_, ticker.mm_max_utilization_,
        ticker.mm_avg_utilization_
    );
}

/*int
Dashboard::displayMarketMakerBotsData(int start_y, int start_x, const TickerState& bots)
{
    for (size_t i = 0; i < bots.mm_open_bids_.size(); i++) {
        mvprintw(start_y, start_x, "Bot: %d", static_cast<int>(i));
        mvprintw(
            start_y + 1, start_x, "Num open bids/asks: %d %d", bots.mm_open_bids_[i],
            bots.mm_open_asks_[i]
        );
        mvprintw(
            start_y + 2, start_x, "$ open bids/asks: %.2f %.2f",
            static_cast<double>(bots.mm_bid_interest_[i]),
            static_cast<double>(bots.mm_ask_interest_[i])
        );
        mvprintw(
            start_y + 3, start_x, "Utilization: %.2f",
            static_cast<double>(bots.mm_utilization_[i])
        );
        start_y += 5;
    }
    return start_y;
}*/

void
Dashboard::clear_screen()
{
    clear();
    refresh();
}

void
Dashboard::mainLoop()
{
    int x_max{};
    int y_max{};
    getmaxyx(stdscr, y_max, x_max);

    clear_screen();
    auto& states = dashboard::DashboardState::get_instance().get_ticker_states();
    size_t num_tickers = states.size();
    drawTickerLayout(num_tickers);

    size_t idx = 0;
    for (auto& [ticker, state] : states) {
        int start_x = (idx++ * x_max / static_cast<int>(num_tickers)) + 2;
        displayStockTickerData(2, start_x, state);
    }
    refresh();
}

} // namespace dashboard
} // namespace nutc
