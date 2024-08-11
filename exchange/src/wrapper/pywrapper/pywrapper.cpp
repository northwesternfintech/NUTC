#include "pywrapper.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#include <iostream>

namespace nutc::pywrapper {

namespace py = pybind11;

void
create_api_module(
    LimitOrderFunction publish_limit_order, MarketOrderFunction publish_market_order
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

    auto sys_modules = sys.attr("modules").cast<py::dict>();
    sys_modules["nutc_api"] = module;

    py::exec(R"(import nutc_api)");
}

OrderBookUpdateFunction
ob_update_function()
{
    return [](const util::position& position) {
        std::string ticker_val{position.ticker};
        std::string side_val = (position.side == util::Side::buy) ? "BUY" : "SELL";
        double price_val{position.price};
        double quantity{position.quantity};
        try {
            py::globals()["strategy"].attr("on_orderbook_update")(
                ticker_val, side_val, quantity, price_val
            );
        } catch (const py::error_already_set& err) {
            std::cerr << err.what() << "\n";
        }
    };
}

TradeUpdateFunction
trade_update_function()
{
    return [](const util::position& position) {
        std::string ticker_val{position.ticker};
        std::string side_val = (position.side == util::Side::buy) ? "BUY" : "SELL";
        double price_val{position.price};
        double quantity{position.quantity};
        try {
            py::globals()["strategy"].attr("on_trade_update")(
                ticker_val, side_val, quantity, price_val
            );
        } catch (const py::error_already_set& err) {
            std::cerr << err.what() << "\n";
        }
    };
}

AccountUpdateFunction
account_update_function()
{
    return [](const util::position& position, double held_capital) {
        std::string ticker_val{position.ticker};
        std::string side_val = (position.side == util::Side::buy) ? "BUY" : "SELL";
        double price_val{position.price};
        double quantity{position.quantity};
        try {
            py::globals()["strategy"].attr("on_account_update")(
                ticker_val, side_val, quantity, price_val, held_capital
            );
        } catch (const py::error_already_set& err) {
            std::cerr << err.what() << "\n";
        }
    };
}

void
run_initialization_code(const std::string& py_code)
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
    py::exec("strategy = Strategy()");
}

} // namespace nutc::pywrapper
