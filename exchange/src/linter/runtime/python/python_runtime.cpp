#include "python_runtime.hpp"

#include <fmt/core.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace nutc::lint {

namespace py = pybind11;

PyRuntime::~PyRuntime()
{
    pybind11::finalize_interpreter();
}

std::optional<std::string>
PyRuntime::init()
{
    auto api_status =
        create_api_module(m_limit_order_func, m_market_order_func, m_cancel_order_func);
    if (!api_status) {
        return "[linter] failed to create API module\n";
    }
    auto init_status = run_initialization_code(algo_);
    if (init_status.has_value()) {
        return init_status.value();
    }

    return std::nullopt;
}

void
PyRuntime::fire_on_trade_update(
    common::Ticker ticker, common::Side side, double price, double quantity
) const
{
    py::globals()["strategy"].attr("on_trade_update")(
        ticker, side, static_cast<double>(quantity), static_cast<double>(price)
    );
}

void
PyRuntime::fire_on_orderbook_update(
    common::Ticker ticker, common::Side side, double price, double quantity
) const
{
    py::globals()["strategy"].attr("on_orderbook_update")(
        ticker, side, static_cast<double>(quantity), static_cast<double>(price)
    );
}

void
PyRuntime::fire_on_account_update(
    common::Ticker ticker, common::Side side, double price, double quantity,
    double capital
) const
{
    py::globals()["strategy"].attr("on_account_update")(
        ticker, side, static_cast<double>(quantity), static_cast<double>(price),
        static_cast<double>(capital)
    );
}

bool
PyRuntime::create_api_module(
    LimitOrderFunction publish_limit_order, MarketOrderFunction publish_market_order,
    CancelOrderFunction cancel_order
)
{
    try {
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

        py::enum_<common::Side>(module, "Side")
            .value("BUY", common::Side::buy)
            .value("SELL", common::Side::sell)
            .export_values();
        py::enum_<common::Ticker>(module, "Ticker")
            .value("ETH", common::Ticker::ETH)
            .value("BTC", common::Ticker::BTC)
            .value("LTC", common::Ticker::LTC)
            .export_values();
        module.def("publish_market_order", publish_market_order);
        module.def("publish_limit_order", publish_limit_order);
        module.def("cancel_order", cancel_order);

        auto sys_modules = sys.attr("modules").cast<py::dict>();
        sys_modules["nutc_api"] = module;

        py::exec(R"(import nutc_api)");
    } catch (...) {
        return false;
    }
    return true;
}

std::optional<std::string>
PyRuntime::run_initialization_code(const std::string& py_code)
{
    py::exec("Side = nutc_api.Side");
    py::exec("Ticker = nutc_api.Ticker");

    try {
        py::exec(py_code);
    } catch (const std::exception& e) {
        return fmt::format("Failed to import code: {}", e.what());
    }

    py::exec("Side = nutc_api.Side");
    py::exec("Ticker = nutc_api.Ticker");
    py::exec(R"(
        def place_market_order(side: Side, ticker: Ticker, quantity: float):
            return nutc_api.publish_market_order(side, ticker, quantity)
    )");
    py::exec(R"(
        def place_limit_order(side: Side, ticker: Ticker, quantity: float, price: float, ioc: bool = False):
            return nutc_api.publish_limit_order(side, ticker, quantity, price, ioc)
    )");
    py::exec(R"(
        def cancel_order(ticker: Ticker, order_id: int):
            return nutc_api.cancel_order(ticker, order_id)
    )");

    py::exec("Side = nutc_api.Side");
    py::exec("Ticker = nutc_api.Ticker");

    try {
        py::exec("strategy = Strategy()");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run initialization: {}", e.what());
    }

    try {
        py::object main_module = py::module_::import("__main__");
        py::dict main_dict = main_module.attr("__dict__");
        py::object on_trade_update = main_dict["strategy"].attr("on_trade_update");
        py::object on_orderbook_update =
            main_dict["strategy"].attr("on_orderbook_update");
        py::object on_account_update = main_dict["strategy"].attr("on_account_update");
    } catch (py::error_already_set& e) {
        return fmt::format("Failed to import callback functions: {}", e.what());
    }

    return std::nullopt;
}

} // namespace nutc::lint
