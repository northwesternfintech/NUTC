#include "dashboard.hpp"

#include "exchange/tick_scheduler/tick_scheduler.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_container.hpp"
#include "exchange/traders/trader_types/algo_trader.hpp"
#include "exchange/traders/trader_types/trader_interface.hpp"
#include "shared/config/config_loader.hpp"
#include "state/global_metrics.hpp"

#include <quill/Quill.h>

#include <cassert>

#include <algorithm>
#include <memory>

#ifdef DASHBOARD
namespace nutc {
namespace dashboard {
Dashboard::Dashboard() :
    err_file_(freopen("logs/error_log.txt", "w", stderr)),
    TICK_HZ(config::Config::get().constants().TICK_HZ),
    DISPLAY_HZ(config::Config::get().constants().DISPLAY_HZ)
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

    main_loop_(0);
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
Dashboard::draw_ticker_layout(WINDOW* window, int start_y, size_t num_tickers)
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
Dashboard::display_stock_ticker_data(
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
    mvwprintw(
        window, start_y++, start_x, "Matches since last tick: %lu",
        ticker.matches_since_last_tick_
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
            window, start_y++, start_x, "%s min/max/avg num bids: %.2f %.2f %.2f",
            bot_type, state.min_open_bids_, state.max_open_bids_, state.avg_open_bids_
        );
        mvwprintw(
            window, start_y++, start_x, "%s min/max/avg num asks: %.2f %.2f %.2f",
            bot_type, state.min_open_asks_, state.max_open_asks_, state.avg_open_asks_
        );
        mvwprintw(
            window, start_y++, start_x, "%s min/max/avg utilization: %.2f %.2f %.2f",
            bot_type, state.min_utilization_, state.max_utilization_,
            state.avg_utilization_
        );
        start_y++;
    };

    display_bot_stats(ticker.mm_state_, "MM");
}

void
Dashboard::display_stock_tickers(WINDOW* window, int start_y)
{
    auto& states = dashboard::DashboardState::get_instance().get_ticker_states();
    size_t num_tickers = states.size();
    draw_ticker_layout(window, start_y, num_tickers);

    size_t idx = 0;
    for (auto& [ticker, state] : states) {
        size_t start_x =
            (idx++ * static_cast<size_t>(window->_maxx) / states.size()) + 3;
        display_stock_ticker_data(
            window, start_y + 2, static_cast<int>(start_x), state
        );
    }
}

void
Dashboard::display_leaderboard(WINDOW* window, int start_y)
{
    mvwprintw(window, start_y, window->_maxx / 2 - 5, "Leaderboard");
    // TODO: use prometheus client

    traders::TraderContainer& client_manager = traders::TraderContainer::get_instance();
    int start_x = 2;
    int orig_start_y = start_y;
    // todo: move to class member variable
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

    auto ordered_traders = client_manager.get_traders();
    std::sort(
        ordered_traders.begin(), ordered_traders.end(),
        [&portfolio_value](const auto& a, const auto& b) {
            return a->get_capital() + portfolio_value(a)
                   > b->get_capital() + portfolio_value(b);
        }
    ); // NOLINT
    for (const auto& trader : ordered_traders) {
        if (!trader->record_metrics())
            continue;
        double capital = trader->get_capital();
        double portfolio = portfolio_value(trader);
        double pnl = capital + portfolio - trader->get_initial_capital();
        auto name = trader->get_display_name();
        mvwprintw(window, start_y++, start_x, "Competitor: %s", name.c_str());
        mvwprintw(window, start_y++, start_x, "  Portfolio Value: %.2f", portfolio);
        mvwprintw(window, start_y++, start_x, "  Capital: %.2f", capital);
        mvwprintw(window, start_y++, start_x, "  PnL: %.2f", pnl);
        if (start_y + 2 >= window->_maxy) {
            start_y = orig_start_y;
            start_x += 60;
        }
    }
}

void
Dashboard::display_performance(WINDOW* window, int start_y)
{
    mvwprintw(window, start_y, window->_maxx / 2 - 5, "Performance");

    ticks::TickJobScheduler& tick_scheduler = ticks::TickJobScheduler::get();
    ticks::TickJobScheduler::tick_metrics_t metrics = tick_scheduler.get_tick_metrics();
    start_y++;
    mvwprintw(
        window, start_y++, window->_maxx / 2 - 8, "Current Tick: %lu",
        tick_scheduler.get_current_tick()
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
        window, start_y++, window->_maxx / 2 - 12, "Theoretical max hz: %.2f",
        1000.0 / static_cast<double>(metrics.avg_tick_ms.count())
    );
}

void
Dashboard::display_log_(WINDOW* window, int start_y)
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
Dashboard::main_loop_(uint64_t tick)
{
    char chr = static_cast<char>(getch());
    if (chr == '1' || chr == '2' || chr == '3' || chr == '4') {
        current_window_ = chr;
    }
    else if (tick % (TICK_HZ / DISPLAY_HZ) != 0) {
        // Only update once per half sec
        return;
    }

    // Hacky way to clear the screen, probably can be removed if we tweak the wrapper?
    if (tick < TICK_HZ * 4 && TICK_HZ * 2 > 100) {
        clear();
        refresh();
    }

    switch (current_window_) {
        case '1':
            calculate_ticker_metrics();
            werase(log_window_);
            werase(leaderboard_window_);
            werase(ticker_window_);
            werase(performance_window_);
            draw_generic_text(ticker_window_, 0);
            display_stock_tickers(ticker_window_, 1);
            wrefresh(ticker_window_);
            break;
        case '2':
            werase(ticker_window_);
            werase(leaderboard_window_);
            werase(log_window_);
            werase(performance_window_);
            draw_generic_text(log_window_, 0);
            display_log_(log_window_, 1);
            wrefresh(log_window_);
            break;
        case '3':
            werase(ticker_window_);
            werase(leaderboard_window_);
            werase(log_window_);
            werase(performance_window_);
            draw_generic_text(leaderboard_window_, 0);
            display_leaderboard(leaderboard_window_, 1);
            wrefresh(leaderboard_window_);
            break;
        case '4':
            werase(ticker_window_);
            werase(log_window_);
            werase(performance_window_);
            werase(leaderboard_window_);
            draw_generic_text(performance_window_, 0);
            display_performance(performance_window_, 2);
            wrefresh(performance_window_);
            break;
        default:
            break;
    }
}

} // namespace dashboard
} // namespace nutc
#endif
