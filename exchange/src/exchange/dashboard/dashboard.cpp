#include "dashboard.hpp"

#include "exchange/traders/trader_manager.hpp"
#include "exchange/traders/trader_types.hpp"
#include "state/global_metrics.hpp"

#include <quill/Quill.h>

#include <cassert>

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

    // make our calls to get input non blocking
    nodelay(stdscr, TRUE);

    int x_max{};
    int y_max{};
    getmaxyx(stdscr, y_max, x_max);
    ticker_window_ = newwin(y_max, x_max, 0, 0);
    log_window_ = newwin(y_max, x_max, 0, 0);
    leaderboard_window_ = newwin(y_max, x_max, 0, 0);

    scrollok(log_window_, TRUE);
    scrollok(log_window_, TRUE);
    scrollok(leaderboard_window_, TRUE);

    werase(ticker_window_);
    werase(leaderboard_window_);
    werase(log_window_);
    wrefresh(ticker_window_);
    wrefresh(leaderboard_window_);
    wrefresh(log_window_);

    mainLoop(0);
}

void
Dashboard::close()
{
    delwin(ticker_window_);
    delwin(log_window_);
    delwin(leaderboard_window_);
    fflush(err_file_);
    fclose(err_file_);
    clear();
    refresh();
    curs_set(0);
    endwin();
}

void
draw_generic_text(WINDOW* window, int start_y)
{
    mvwprintw(
        window, start_y++, window->_maxx / 2 - 38,
        "Press '1' for Ticker Window, '2' for Log Window, '3' for Leaderboard Window"
    );
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

    attron(COLOR_PAIR(2));
    mvwprintw(window, start_y, x_max / 2 - 3, "Tickers");
    attroff(COLOR_PAIR(2));
}

void
Dashboard::displayStockTickerData(
    WINDOW* window, int start_y, int start_x, const TickerState& ticker
)
{
    mvwprintw(window, start_y++, start_x, "Ticker: %s", ticker.TICKER.c_str());
    mvwprintw(
        window, start_y++, start_x, "Midprice: %.2f",
        static_cast<double>(ticker.midprice_)
    );
    mvwprintw(
        window, start_y++, start_x, "Theo Price: %.2f",
        static_cast<double>(ticker.theo_)
    );
    mvwprintw(
        window, start_y++, start_x, "Spread: %.2f %.2f",
        static_cast<double>(ticker.spread_.first),
        static_cast<double>(ticker.spread_.second)
    );
    mvwprintw(
        window, start_y++, start_x, "Bids/asks: %lu %lu", ticker.num_bids_,
        ticker.num_asks_
    );
    mvwprintw(window, start_y++, start_x, "MM bots: %lu", ticker.num_mm_bots_);
    mvwprintw(
        window, start_y++, start_x, "MM min/max/avg num bids: %lu %lu %lu",
        ticker.mm_min_open_bids_, ticker.mm_max_open_bids_, ticker.mm_avg_open_bids_
    );
    mvwprintw(
        window, start_y++, start_x, "MM min/max/avg num asks: %lu %lu %lu",
        ticker.mm_min_open_asks_, ticker.mm_max_open_asks_, ticker.mm_avg_open_asks_
    );
    mvwprintw(
        window, start_y++, start_x, "MM min/max/avg utilization: %.2f %.2f %.2f",
        static_cast<double>(ticker.mm_min_utilization_),
        static_cast<double>(ticker.mm_max_utilization_),
        static_cast<double>(ticker.mm_avg_utilization_)
    );

    start_y++;
    mvwprintw(window, start_y++, start_x, "Retail bots: %lu", ticker.num_retail_bots_);
    mvwprintw(
        window, start_y++, start_x, "Retail min/max/avg num bids: %lu %lu %lu",
        ticker.retail_min_open_bids_, ticker.retail_max_open_bids_,
        ticker.retail_avg_open_bids_
    );
    mvwprintw(
        window, start_y++, start_x, "Retail min/max/avg num asks: %lu %lu %lu",
        ticker.retail_min_open_asks_, ticker.retail_max_open_asks_,
        ticker.retail_avg_open_asks_
    );
    mvwprintw(
        window, start_y++, start_x, "Retail min/max/avg utilization: %.2f %.2f %.2f",
        static_cast<double>(ticker.retail_min_utilization_),
        static_cast<double>(ticker.retail_max_utilization_),
        static_cast<double>(ticker.retail_avg_utilization_)
    );
    mvwprintw(
        window, start_y++, start_x, "Retail min/max/avg PnL: %.2f %.2f %.2f",
        static_cast<double>(ticker.retail_min_pnl_),
        static_cast<double>(ticker.retail_max_pnl_),
        static_cast<double>(ticker.retail_avg_pnl_)
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
Dashboard::displayLeaderboard(WINDOW* window, int start_y)
{
    mvwprintw(window, start_y, window->_maxx / 2 - 5, "Leaderboard");

    manager::ClientManager& client_manager = manager::ClientManager::get_instance();
    for (auto& [user_id, trader_variant] : client_manager.get_clients_const()) {
        const manager::generic_trader_t& trader = std::visit(
            [](auto&& arg) -> const manager::generic_trader_t& { return arg; },
            trader_variant
        ); // NOLINT
        mvwprintw(window, start_y++, 2, "User: %s", user_id.c_str());
        mvwprintw(window, start_y++, 2, "  Capital: %.2f", trader.get_capital());
    }
}

void
Dashboard::displayLog(WINDOW* window, int start_y)
{
    mvwprintw(window, start_y, window->_maxx / 2 - 2, "Logs");
    std::string line;
    log_file_ = std::ifstream("logs/app.log", std::ios::in);

    if (log_file_.eof()) {
        log_file_.clear();
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

    // this is a bit hacky, but prevents the log file from getting too big
    std::ofstream("logs/app.log");
}

void
Dashboard::mainLoop(uint64_t tick)
{
    char chr = getch();
    if (chr == '1' || chr == '2' || chr == '3')
        current_window_ = chr;
    else if (tick % 4 != 0)
        return;

    switch (current_window_) {
        case '1':
            werase(log_window_);
            werase(leaderboard_window_);
            draw_generic_text(ticker_window_, 0);
            displayStockTickers(ticker_window_, 1);
            wrefresh(ticker_window_);
            break;
        case '2':
            werase(ticker_window_);
            werase(leaderboard_window_);
            draw_generic_text(log_window_, 0);
            displayLog(log_window_, 1);
            wrefresh(log_window_);
            break;
        case '3':
            werase(ticker_window_);
            werase(log_window_);
            draw_generic_text(leaderboard_window_, 0);
            displayLeaderboard(leaderboard_window_, 1);
            wrefresh(leaderboard_window_);
            break;
        default:
            break;
    }
}

} // namespace dashboard
} // namespace nutc
