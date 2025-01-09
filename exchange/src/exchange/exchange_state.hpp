#pragma once

#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/orders/ticker_container.hpp"
#include "exchange/traders/trader_container.hpp"

namespace nutc::exchange {
struct exchange_state {
    TraderContainer traders;
    TickerContainer tickers;

    explicit exchange_state(const std::vector<ticker_config>& configs) :
        tickers{configs, traders}
    {}

    exchange_state() : tickers{{}, traders} {}
};
} // namespace nutc::exchange
