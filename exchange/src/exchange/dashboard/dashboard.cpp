#include "dashboard.hpp"

#include "state/global_metrics.hpp"

#include <cassert>

#include <quill/Quill.h>

namespace nutc {
namespace dashboard {

Dashboard::Dashboard() : err_file_(freopen("logs/error_log.txt", "w", stderr))
{
    quill::stdout_handler("console")->set_log_level(quill::LogLevel::Error);
    std::ofstream create_file("logs/app.log");
    create_file.close();
    log_file_ = std::ifstream("logs/app.log", std::ios::in);
    if (!log_file_.is_open()) {
        printw("Failed to open file\n");
        close();
        std::abort();
    }

    initscr();
    noecho();
    curs_set(0);
    cbreak();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    int x_max{};
    int y_max{};
    getmaxyx(stdscr, y_max, x_max);
    ticker_window_ = newwin(y_max, x_max, 0, 0);
    log_window_ = newwin(y_max, x_max, 0, 0);

    scrollok(log_window_, TRUE);
    scrollok(log_window_, TRUE);

    werase(ticker_window_);
    werase(log_window_);
    wrefresh(ticker_window_);
    wrefresh(log_window_);

    mainLoop(0);
}

void
Dashboard::close()
{
    delwin(ticker_window_);
    delwin(log_window_);
    fflush(err_file_);
    fclose(err_file_);
    clear();
    refresh();
    curs_set(0);
    endwin();
}

void
Dashboard::drawTickerLayout(WINDOW* window, int start_y, size_t num_tickers)
{
    assert(num_tickers <= 4);

    int x_max{};
    int y_max{};
    getmaxyx(stdscr, y_max, x_max);

    for (size_t i = 1; i < num_tickers + 1; ++i) {
        for (int j = start_y + 2; j < y_max; ++j) {
            mvwprintw(window, j, i * x_max / num_tickers, "|"); // NOLINT
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
    mvwprintw(
        window, start_y, x_max / 2 - 22, "Stock Ticker Window - Press 2 for Log Window"
    );
    attroff(COLOR_PAIR(1));
}

void
Dashboard::displayStockTickerData(
    WINDOW* window, int start_y, int start_x, const TickerState& ticker
)
{
    mvwprintw(window, start_y, start_x, "Ticker: %s", ticker.TICKER.c_str());
    mvwprintw(
        window, start_y + 2, start_x, "Midprice: %.2f",
        static_cast<double>(ticker.midprice_)
    );
    mvwprintw(
        window, start_y + 3, start_x, "Theo Price: %.2f",
        static_cast<double>(ticker.theo_)
    );
    mvwprintw(
        window, start_y + 4, start_x, "Spread: %.2f %.2f",
        static_cast<double>(ticker.spread_.first),
        static_cast<double>(ticker.spread_.second)
    );
    mvwprintw(
        window, start_y + 5, start_x, "Bids/asks: %lu %lu", ticker.num_bids_,
        ticker.num_asks_
    );
    mvwprintw(window, start_y + 7, start_x, "MM bots: %lu", ticker.num_mm_bots_);
    mvwprintw(
        window, start_y + 8, start_x, "MM min/max/avg num bids: %lu %lu %lu",
        ticker.mm_min_open_bids_, ticker.mm_max_open_bids_, ticker.mm_avg_open_bids_
    );
    mvwprintw(
        window, start_y + 9, start_x, "MM min/max/avg num asks: %lu %lu %lu",
        ticker.mm_min_open_asks_, ticker.mm_max_open_asks_, ticker.mm_avg_open_asks_
    );
    mvwprintw(
        window, start_y + 10, start_x, "MM min/max/avg utilization: %.2f %.2f %.2f",
        static_cast<double>(ticker.mm_min_utilization_),
        static_cast<double>(ticker.mm_max_utilization_),
        static_cast<double>(ticker.mm_avg_utilization_)
    );
}

void
Dashboard::displayStockTickers(WINDOW* window, int start_y)
{
    auto& states = dashboard::DashboardState::get_instance().get_ticker_states();
    size_t num_tickers = states.size();
    drawTickerLayout(window, start_y, num_tickers);

    size_t idx = 0;
    for (auto& [ticker, state] : states) {
        size_t start_x =
            (idx++ * static_cast<size_t>(window->_maxx) / states.size()) + 3;
        displayStockTickerData(window, start_y + 2, static_cast<int>(start_x), state);
    }
}

void
Dashboard::displayLog(WINDOW* window, int start_y)
{
    mvwprintw(
        window, start_y, window->_maxx / 2 - 20,
        "Log Window - Press '1' for Ticker Window"
    );
    std::string line;
    log_file_ = std::ifstream("logs/app.log", std::ios::in);

    if (log_file_.eof()) {
        log_file_.clear();
    } else if (log_file_.fail() || log_file_.bad()) {
    std::abort();
    }
  
    int y = start_y + 2;
    while (std::getline(log_file_, line)) {
        log_queue_.push_back(line);
    }
    while (log_queue_.size() > (window->_maxy - start_y - 1)) {
        log_queue_.pop_front();
    }

    for (const std::string& line : log_queue_) {
        mvwprintw(window, y++, 2, line.c_str());
    }
}

void
Dashboard::mainLoop(uint64_t tick)
{
    char chr = getch();
    if (chr == '1' || chr == '2')
        current_window_ = chr;
    else if (tick % 4 != 0)
        return;

    switch (current_window_) {
        case '1':
            werase(log_window_);
            displayStockTickers(ticker_window_, 1);
            wrefresh(ticker_window_);
            break;
        case '2':
            werase(ticker_window_);
            displayLog(log_window_, 1);
            wrefresh(log_window_);
            break;
        default:
            break;
    }
}

} // namespace dashboard
} // namespace nutc
