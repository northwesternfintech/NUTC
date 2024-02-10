#pragma once

#include "exchange/dashboard/state/ticker_state.hpp"
#include "exchange/tick_manager/tick_observer.hpp"

namespace nutc {
namespace dashboard {

// NOLINTBEGIN

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
    void close();
    void init();
    void clear_screen();

private:
    void drawTickerLayout(size_t num_tickers);

    void displayStockTickerData(int start_y, int start_x, const TickerState& ticker);

    int displayMarketMakerBotsData(int start_y, int start_x, const TickerState& bots);

    void mainLoop();
    
    FILE* err_file_;

    Dashboard() = default;
    ~Dashboard() = default;
};

} // namespace dashboard
} // namespace nutc

//
//
// NOLINTEND
