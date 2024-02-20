#include "dashboard.hpp"

#include "exchange/tick_manager/tick_manager.hpp"
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
    performance_window_ = newwin(y_max, x_max, 0, 0);

    scrollok(ticker_window_, TRUE);
    scrollok(log_window_, TRUE);
    scrollok(leaderboard_window_, TRUE);
    scrollok(performance_window_, TRUE);

    werase(ticker_window_);
    werase(leaderboard_window_);
    werase(performance_window_);
    werase(log_window_);

    wrefresh(ticker_window_);
    wrefresh(leaderboard_window_);
    wrefresh(log_window_);
    wrefresh(performance_window_);

    mainLoop(0);
}

void
Dashboard::close()
{
    delwin(ticker_window_);
    delwin(log_window_);
    delwin(leaderboard_window_);
    delwin(performance_window_);
    fflush(err_file_); // NOLINT
    fclose(err_file_); // NOLINT
    clear();
    refresh();
    curs_set(0);
    endwin();
}

void
draw_generic_text(WINDOW* window, int start_y)
{
    mvwprintw(
        window, start_y++, window->_maxx / 2 - 52,
        "Press '1' for Ticker Window, '2' for Log Window, '3' for Leaderboard Window, "
        "'4' for Performance Window"
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
            mvwprintw(
                window, j, static_cast<int>(i) * x_max / static_cast<int>(num_tickers),
                "|"
            ); // NOLINT
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
    mvwprintw(window, start_y++, start_x, "Midprice: %.2f", ticker.midprice_);
    mvwprintw(window, start_y++, start_x, "Theo Price: %.2f", ticker.theo_);
    mvwprintw(
        window, start_y++, start_x, "Spread: %.2f %.2f", ticker.spread_.first,
        ticker.spread_.second
    );
    mvwprintw(
        window, start_y++, start_x, "Bids/asks: %lu %lu", ticker.num_bids_,
        ticker.num_asks_
    );
    start_y++;

    auto display_bot_stats = [&](const BotStates& state, const char* bot_type) {
        mvwprintw(
            window, start_y++, start_x, "%s bots: %lu", bot_type, state.num_bots_
        );
        mvwprintw(
            window, start_y++, start_x, "%s active bots: %lu - %.2f%%", bot_type,
            state.num_bots_active_, state.percent_active_
        );
        mvwprintw(
            window, start_y++, start_x, "%s total capital held: %.0f", bot_type,
            state.total_capital_held_
        );
        mvwprintw(
            window, start_y++, start_x, "%s min/max/avg num bids: %lu %lu %.2f",
            bot_type, state.min_open_bids_, state.max_open_bids_, state.avg_open_bids_
        );
        mvwprintw(
            window, start_y++, start_x, "%s min/max/avg num asks: %lu %lu %.2f",
            bot_type, state.min_open_asks_, state.max_open_asks_, state.avg_open_asks_
        );
        mvwprintw(
            window, start_y++, start_x, "%s min/max/avg utilization: %.2f %.2f %.2f",
            bot_type, state.min_utilization_, state.max_utilization_,
            state.avg_utilization_
        );
        mvwprintw(
            window, start_y++, start_x, "%s min/max/avg PnL: %.2f %.2f %.2f", bot_type,
            state.min_pnl_, state.max_pnl_, state.avg_pnl_
        );
        start_y++;
    };

    display_bot_stats(ticker.mm_state_, "MM");
    display_bot_stats(ticker.retail_state_, "Retail");
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
    for (const auto& [user_id, trader] : client_manager.get_traders()) {
        mvwprintw(window, start_y++, 2, "User: %s", trader->get_id().c_str());
        mvwprintw(window, start_y++, 2, "  Capital: %.2f", trader->get_capital());
    }
}

void
Dashboard::displayPerformance(WINDOW* window, int start_y)
{
    mvwprintw(window, start_y, window->_maxx / 2 - 5, "Performance");

    ticks::TickManager& tick_manager = ticks::TickManager::get_instance();
    ticks::TickManager::tick_metrics_t metrics = tick_manager.get_tick_metrics();
    start_y++;
    if (tick_manager.get_current_tick() < 100) {
        mvwprintw(
            window, start_y + 4, window->_maxx / 2 - 23,
            "Current tick (%lu) below 100. Not enough data",
            tick_manager.get_current_tick()
        );
        return;
    }
    mvwprintw(
        window, start_y++, window->_maxx / 2 - 8, "Current Tick: %lu",
        tick_manager.get_current_tick()
    );
    mvwprintw(
        window, start_y++, window->_maxx / 2 - 13, "Top 1p tick times(ms): %lu",
        metrics.top_1p_ms.count()
    );
    mvwprintw(
        window, start_y++, window->_maxx / 2 - 13, "Top 5p tick times(ms): %lu",
        metrics.top_5p_ms.count()
    );
    mvwprintw(
        window, start_y++, window->_maxx / 2 - 13, "Top 10p tick times(ms): %lu",
        metrics.top_10p_ms.count()
    );
    mvwprintw(
        window, start_y++, window->_maxx / 2 - 13, "Top 50p tick times(ms): %lu",
        metrics.top_50p_ms.count()
    );
    mvwprintw(
        window, start_y++, window->_maxx / 2 - 13, "Average tick time(ms): %lu",
        metrics.avg_tick_ms.count()
    );
    mvwprintw(
        window, start_y++, window->_maxx / 2 - 12, "Median tick time(ms): %lu",
        metrics.median_tick_ms.count()
    );
    mvwprintw(
        window, start_y++, window->_maxx / 2 - 12, "Theoretical max hz: %.2f",
        1000.0 / static_cast<double>(metrics.avg_tick_ms.count())
    );
}

void
Dashboard::displayLog(WINDOW* window, int start_y)
{
    mvwprintw(window, start_y, window->_maxx / 2 - 2, "Logs");
    std::string line;

    if (log_file_.eof()) {
        log_file_.clear();
    }

    int curr_y = start_y + 3;
    while (std::getline(log_file_, line)) {
        log_queue_.push_back(line);
        if (log_file_.peek() == EOF)
            break;
    }

    while (static_cast<int>(log_queue_.size()) > (window->_maxy - start_y - 1)) {
        log_queue_.pop_front();
    }

    for (const std::string& line : log_queue_) {
        mvwprintw(window, curr_y++, 2, "%s", line.c_str());
    }
    // this is a bit hacky, but prevents the log file from getting too big
    // std::ofstream("logs/app.log");
}

void
Dashboard::calculate_ticker_metrics()
{
    auto& states = dashboard::DashboardState::get_instance().get_ticker_states();
    for (auto& [_, state] : states) {
        state.calculate_metrics();
    }
}

void
Dashboard::mainLoop(uint64_t tick)
{
    char chr = static_cast<char>(getch());
    if (chr == '1' || chr == '2' || chr == '3' || chr == '4')
        current_window_ = chr;
    else if (tick % 15 != 0)
        return;

    switch (current_window_) {
        case '1':
            calculate_ticker_metrics();
            werase(log_window_);
            werase(leaderboard_window_);
            werase(ticker_window_);
            werase(performance_window_);
            draw_generic_text(ticker_window_, 0);
            displayStockTickers(ticker_window_, 1);
            wrefresh(ticker_window_);
            break;
        case '2':
            werase(ticker_window_);
            werase(leaderboard_window_);
            werase(log_window_);
            werase(performance_window_);
            draw_generic_text(log_window_, 0);
            displayLog(log_window_, 1);
            wrefresh(log_window_);
            break;
        case '3':
            werase(ticker_window_);
            werase(leaderboard_window_);
            werase(log_window_);
            werase(performance_window_);
            draw_generic_text(leaderboard_window_, 0);
            displayLeaderboard(leaderboard_window_, 1);
            wrefresh(leaderboard_window_);
            break;
        case '4':
            werase(ticker_window_);
            werase(log_window_);
            werase(performance_window_);
            werase(leaderboard_window_);
            draw_generic_text(performance_window_, 0);
            displayPerformance(performance_window_, 2);
            wrefresh(performance_window_);
            break;
        default:
            break;
    }
}

} // namespace dashboard
} // namespace nutc
