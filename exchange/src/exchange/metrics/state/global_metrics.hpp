#pragma once

#include "ticker_state.hpp"

#include <string>
#include <unordered_map>

namespace nutc {
namespace dashboard {

class DashboardState {
    std::unordered_map<std::string, TickerState> ticker_states_;

public:
    void
    add_ticker(std::string ticker, double starting_price)
    {
        ticker_states_.emplace(ticker, TickerState{ticker, starting_price});
    }

    TickerState&
    get_ticker_state(const std::string& ticker)
    {
        return ticker_states_.at(ticker);
    }

    std::unordered_map<std::string, TickerState>&
    get_ticker_states()
    {
        return ticker_states_;
    }

    static DashboardState&
    get_instance()
    {
        static DashboardState instance;
        return instance;
    }

    DashboardState(const DashboardState&) = delete;
    DashboardState& operator=(const DashboardState&) = delete;
    DashboardState(DashboardState&&) = delete;
    DashboardState& operator=(DashboardState&&) = delete;

    ~DashboardState() = default;

private:
    DashboardState() = default;
};

} // namespace dashboard
} // namespace nutc
