#pragma once

#include "common/types/decimal.hpp"
#include "common/util.hpp"
#include "wrapper/messaging/exchange_communicator.hpp"
#include "wrapper/runtime/runtime.hpp"

#include <pybind11/embed.h>

namespace nutc::wrapper {

#pragma GCC visibility push(hidden)

class PyRuntime : public Runtime {
public:
    PyRuntime(std::string algo, std::string trader_id) :
        Runtime(std::move(algo), std::move(trader_id))
    {
        create_api_module(
            communicator_.place_limit_order(), communicator_.place_market_order(),
            communicator_.cancel_order()
        );
        run_initialization_code(algo_);
    }

private:
    pybind11::scoped_interpreter guard_;

    void fire_on_trade_update(
        Ticker ticker, Side side, decimal_quantity quantity, decimal_price price
    ) const override;

    void fire_on_orderbook_update(
        Ticker ticker, Side side, decimal_quantity quantity, decimal_price price
    ) const override;

    void fire_on_account_update(
        Ticker ticker, Side side, decimal_price price, decimal_quantity quantity,
        decimal_price capital
    ) const override;

    static void create_api_module(
        LimitOrderFunction publish_limit_order,
        MarketOrderFunction publish_market_order, CancelOrderFunction cancel_order
    );
    static void run_initialization_code(const std::string& py_code);
};

#pragma GCC visibility pop

} // namespace nutc::wrapper
