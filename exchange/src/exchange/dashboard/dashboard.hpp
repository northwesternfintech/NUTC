#pragma once

#include "exchange/dashboard/state/global_metrics.hpp"
#include "exchange/dashboard/state/ticker_state.hpp"
#include "exchange/logging.hpp"
#include "exchange/tick_manager/tick_observer.hpp"

#include <ncurses.h>
#include <unistd.h> // For sleep/usleep

namespace nutc {
namespace dashboard {

// NOLINTBEGIN

struct MarketMakerBot {
    float longCapital = 50000.0;
    float shortCapital = 30000.0;
    // Add more fields as necessary
};

void
drawLayout()
{
    int xMax, yMax;
    getmaxyx(stdscr, yMax, xMax);

    int middle = xMax / 2;
    // Draw a vertical line in the middle
    for (int i = 0; i < yMax; ++i) {
        mvprintw(i, middle, "|");
    }

    // Optional: Set different colors for each pane
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
    }
    attron(COLOR_PAIR(1));
    mvprintw(0, 1, "Stock Ticker Pane");
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(2));
    mvprintw(0, middle + 1, "Market Maker Bot Pane");
    attroff(COLOR_PAIR(2));
}

int
displayStockTickerData(int startY, int startX, const TickerState& ticker)
{
    mvprintw(startY, startX, "Ticker: %s", ticker.TICKER.c_str());
    mvprintw(startY + 1, startX, "Num MM Bots: %d", ticker.num_mm_bots_);
    mvprintw(startY + 2, startX, "Midprice: %.2f", ticker.midprice_);
    mvprintw(startY + 3, startX, "Theo Price: %.2f", ticker.theo_);
    mvprintw(
        startY + 4, startX, "Spread: %.2f %.2f", ticker.spread_.first,
        ticker.spread_.second
    );
    mvprintw(
        startY + 5, startX, "Bids/asks: %d %d", ticker.num_bids_, ticker.num_asks_
    );
    return startY + 7;
}

void
displayMarketMakerBotData(int startY, int startX, const MarketMakerBot& bot)
{
    mvprintw(startY, startX, "Long Capital: %.2f", bot.longCapital);
    mvprintw(startY + 1, startX, "Short Capital: %.2f", bot.shortCapital);
    // Add more metrics here
}

void
mainLoop()
{
    int xMax, yMax;
    getmaxyx(stdscr, yMax, xMax);
    int middle = xMax / 2;

    MarketMakerBot bot;

    // Clear previous content
    clear();
    // Redraw layout to avoid overwriting the separators
    drawLayout();
    // Display updated data

    int starty = 1;
    for (auto& [ticker, state] :
         dashboard::DashboardState::get_instance().get_ticker_states()) {
        starty = displayStockTickerData(starty, 1, state);
    }
    displayMarketMakerBotData(1, middle + 2, bot);
    // Refresh the screen to show updates
    refresh();
}

class Dashboard : public ticks::TickObserver {
public:
    void
    on_tick(uint64_t tick) override
    {
        if (tick % 4 != 0)
            return;
        mainLoop();
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

private:
    Dashboard() = default;
    ~Dashboard() = default;
};

void
init()
{
    // Initialize ncurses
    initscr();
    noecho();
    curs_set(0); // Hide the cursor

    // Draw initial layout
    drawLayout();

    // Enter the main loop
    mainLoop();
}

void
close()
{
    endwin();
}
} // namespace dashboard
} // namespace nutc

//
//
// NOLINTEND
