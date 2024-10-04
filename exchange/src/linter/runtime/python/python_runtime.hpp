#pragma once

#include "linter/runtime/runtime.hpp"

#include <pybind11/embed.h>

namespace nutc::lint {

class PyRuntime : public Runtime {
public:
    PyRuntime(
        std::string algo, LimitOrderFunction place_limit_order,
        MarketOrderFunction place_market_order, CancelOrderFunction cancel_order
    ) :
        Runtime(
            std::move(algo), std::move(place_limit_order),
            std::move(place_market_order), std::move(cancel_order)
        )
    {
        pybind11::initialize_interpreter();
    }

    ~PyRuntime() override;

    std::optional<std::string> init() override;

    void fire_on_trade_update(
        common::Ticker ticker, common::Side side, float price, float quantity
    ) const override;

    void fire_on_orderbook_update(
        common::Ticker ticker, common::Side side, float price, float quantity
    ) const override;

    void fire_on_account_update(
        common::Ticker ticker, common::Side side, float price, float quantity,
        float capital
    ) const override;

private:
    static bool create_api_module(
        LimitOrderFunction publish_limit_order,
        MarketOrderFunction publish_market_order, CancelOrderFunction cancel_order
    );
    static std::optional<std::string> run_initialization_code(const std::string& py_code
    );
};

} // namespace nutc::lint
