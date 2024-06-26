#include "runtime.hpp"

#include <fmt/format.h>
#include <pybind11/embed.h>

namespace py = pybind11;

namespace nutc {
namespace pywrapper {

bool
create_api_module(
    std::function<bool(const std::string&, const std::string&, float, float)>
        publish_limit_order,
    std::function<bool(const std::string&, const std::string&, float)>
        publish_market_order
)
{
    try {
        py::module m = py::module::create_extension_module(
            "nutc_api", "Official NUTC Exchange API", new py::module::module_def
        );

        m.def("publish_limit_order", publish_limit_order);
        m.def("publish_market_order", publish_market_order);

        py::module_ sys = py::module_::import("sys");
        py::dict sys_modules = sys.attr("modules").cast<py::dict>();
        sys_modules["nutc_api"] = m;

        py::exec(R"(import nutc_api)");
    } catch (...) {
        return false;
    }
    return true;
}

bool
supress_stdout()
{
    try {
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
    } catch (...) {
        return false;
    }
    return true;
}

std::optional<std::string>
import_py_code(const std::string& code)
{
    try {
        py::exec(code);
    } catch (const std::exception& e) {
        return fmt::format("Failed to import code: {}", e.what());
    }
    py::exec(R"(
        def place_market_order(side, ticker, quantity):
            return nutc_api.publish_market_order(side, ticker, quantity))");
    py::exec(R"(
        def place_limit_order(side, ticker, quantity, price):
            return nutc_api.publish_limit_order(side, ticker, price, quantity))");

    return std::nullopt;
}

std::optional<std::string>
run_initialization()
{
    try {
        py::exec("strategy = Strategy()");
    }

    catch (const std::exception& e) {
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

std::optional<std::string>
trigger_callbacks()
{
    try {
        py::exec(R"(place_limit_order("BUY", "ETH", 1.0, 1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run place_limit_order: {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_orderbook_update("ETH","BUY",1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_orderbook_update: {}", e.what());
    }
    try {
        py::exec(R"(strategy.on_trade_update("ETH","BUY",1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_trade_update: {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_account_update("ETH","BUY",1.0,1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_account_update: {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_orderbook_update("BTC","BUY",1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_orderbook_update: {}", e.what());
    }
    try {
        py::exec(R"(strategy.on_trade_update("BTC","BUY",1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_trade_update: {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_account_update("BTC","BUY",1.0,1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_account_update: {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_orderbook_update("LTC","BUY",1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_orderbook_update: {}", e.what());
    }
    try {
        py::exec(R"(strategy.on_trade_update("LTC","BUY",1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_trade_update: {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_account_update("LTC","BUY",1.0,1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_account_update: {}", e.what());
    }

    return std::nullopt;
}

} // namespace pywrapper
} // namespace nutc
