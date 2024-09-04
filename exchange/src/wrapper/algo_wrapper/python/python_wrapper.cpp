#include "python_wrapper.hpp"

#include "wrapper/messaging/exchange_communicator.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#include <iostream>

namespace nutc::wrapper {

namespace py = pybind11;

void
PyWrapper::fire_on_trade_update(
    Ticker ticker, Side side, decimal_price price, decimal_quantity quantity
) const
{
    std::string ticker_val{to_string(ticker)};
    std::string side_val = (side == Side::buy) ? "BUY" : "SELL";

    try {
        py::globals()["strategy"].attr("on_trade_update")(
            ticker_val, side_val, static_cast<double>(quantity),
            static_cast<double>(price)
        );
    } catch (const py::error_already_set& err) {
        std::cerr << err.what() << "\n";
    }
}

void
PyWrapper::fire_on_orderbook_update(
    Ticker ticker, Side side, decimal_price price, decimal_quantity quantity
) const
{
    std::string ticker_val{to_string(ticker)};
    std::string side_val = (side == Side::buy) ? "BUY" : "SELL";
    try {
        py::globals()["strategy"].attr("on_orderbook_update")(
            ticker_val, side_val, static_cast<double>(quantity),
            static_cast<double>(price)
        );
    } catch (const py::error_already_set& err) {
        std::cerr << err.what() << "\n";
    }
}

void
PyWrapper::fire_on_account_update(
    Ticker ticker, Side side, decimal_price price, decimal_quantity quantity,
    decimal_price capital
) const
{
    std::string ticker_val{to_string(ticker)};
    std::string side_val = (side == Side::buy) ? "BUY" : "SELL";
    try {
        py::globals()["strategy"].attr("on_account_update")(
            ticker_val, side_val, static_cast<double>(quantity),
            static_cast<double>(price), static_cast<double>(capital)
        );
    } catch (const py::error_already_set& err) {
        std::cerr << err.what() << "\n";
    }
}

void
PyWrapper::create_api_module(
    LimitOrderFunction publish_limit_order, MarketOrderFunction publish_market_order,
    CancelOrderFunction cancel_order
)
{
    py::module_ sys = py::module_::import("sys");
    // TODO: disable this for testing
    py::exec(R"(
    import sys
    import os
    class SuppressOutput(object):
        def write(self, txt):
            pass
        def flush(self):
            pass

    sys.stdout = SuppressOutput()
    )");
    py::module module = py::module::create_extension_module(
        "nutc_api", "NUTC Exchange API", new py::module::module_def
    );

    module.def("publish_market_order", publish_market_order);
    module.def("publish_limit_order", publish_limit_order);
    module.def("cancel_order", cancel_order);

    auto sys_modules = sys.attr("modules").cast<py::dict>();
    sys_modules["nutc_api"] = module;

    py::exec(R"(import nutc_api)");
}

void
PyWrapper::run_initialization_code(const std::string& py_code)
{
    py::exec(py_code);
    py::exec(R"(
        def place_market_order(side: str, ticker: str, quantity: float):
            return nutc_api.publish_market_order(side, ticker, quantity)
    )");
    py::exec(R"(
        def place_limit_order(side: str, ticker: str, quantity: float, price: float, ioc: bool = False):
            return nutc_api.publish_limit_order(side, ticker, quantity, price, ioc)
    )");
    py::exec(R"(
        def cancel_order(ticker: str, order_id: int):
            return nutc_api.cancel_order(ticker, order_id)
    )");
    py::exec("strategy = Strategy()");
}

} // namespace nutc::wrapper
