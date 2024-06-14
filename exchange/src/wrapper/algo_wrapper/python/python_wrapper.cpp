#include "wrapper/algo_wrapper/python/python_wrapper.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace nutc {
namespace wrapper {

PyWrapper::PyWrapper(
    std::string algo, std::string trader_id, MarketOrderFunc publish_market_order
) : Wrapper(std::move(algo), std::move(trader_id))
{
    py::module_ sys = py::module_::import("sys");
    py::exec(R"(
    import sys
    import os
    class SuppressOutput(object):
        def write(self, txt):
            pass  # Do nothing on write
        def flush(self):
            pass  # Do nothing on flush

    sys.stdout = SuppressOutput()
    )");
    py::module module = py::module::create_extension_module(
        "nutc_api", "NUTC Exchange API", new py::module::module_def
    );
    module.def("publish_market_order", publish_market_order);

    auto sys_modules = sys.attr("modules").cast<py::dict>();
    sys_modules["nutc_api"] = module;

    py::exec(R"(import nutc_api)");

    py::exec(algo_);
    py::exec(R"(
        def place_market_order(side, ticker, quantity, price):
            return nutc_api.publish_market_order(side, ticker, quantity, price)
    )");
    py::exec("strategy = Strategy()");
}

void
PyWrapper::fire_on_trade_update(
    std::string ticker, std::string side, double price, double quantity
) const
{
    py::globals()["strategy"].attr("on_trade_update")(ticker, side, price, quantity);
}

void
PyWrapper::fire_on_orderbook_update(
    std::string ticker, std::string side, double price, double quantity
) const
{
    py::globals()["strategy"].attr("on_orderbook_update")(
        ticker, side, price, quantity
    );
}

void
PyWrapper::fire_on_account_update(
    std::string ticker, std::string side, double price, double quantity,
    double buyer_capital
) const
{
    py::globals()["strategy"].attr("on_account_update")(
        ticker, side, price, quantity, buyer_capital
    );
}
} // namespace wrapper
} // namespace nutc
