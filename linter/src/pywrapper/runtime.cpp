#include "runtime.hpp"

namespace py = pybind11;

namespace nutc {
namespace pywrapper {

bool
create_api_module(
    std::function<bool(const std::string&, const std::string&, float, float)>
        publish_market_order
)
{
    try {
        py::module m = py::module::create_extension_module(
            "nutc_api", "Official NUTC Exchange API", new py::module::module_def
        );
        m.def("publish_market_order", publish_market_order);

        py::module_ sys = py::module_::import("sys");
        py::dict sys_modules = sys.attr("modules").cast<py::dict>();
        sys_modules["nutc_api"] = m;

        py::exec(R"(import nutc_api)");
    } catch (const std::exception& e) {
        return false;
    }
    return true;
}

std::optional<std::string>
import_py_code(const std::string& code)
{
    log_i(mock_runtime, "Importing algorithm code into python interpreter");
    try {
        py::exec(code);
    } catch (const std::exception& e) {
        return fmt::format("Failed to import code: {}", e.what());
    }
    py::exec(R"(
        def place_market_order(side, ticker, quantity, price):
            nutc_api.publish_market_order(side, ticker, quantity, price))");

    return std::nullopt;
}

std::optional<std::string>
run_initialization()
{
    log_i(mock_runtime, "Running initialization code");
    try {
        py::exec("strategy = Strategy()");
    }

    catch (const std::exception& e) {
        return fmt::format("Failed to run initialization: {}", e.what());
    }

    try {
        py::object main_module = py::module_::import("__main__");
        py::dict main_dict = main_module.attr("__dict__");
        py::object on_trade_and_account_update = main_dict["strategy"].attr("on_trade_and_account_update");
        py::object on_orderbook_update = main_dict["strategy"].attr("on_orderbook_update");
        py::object on_holding_change = main_dict["strategy"].attr("on_holding_change");
    } catch (py::error_already_set& e) {
        return fmt::format("Failed to import callback functions: {}", e.what());
    }

    return std::nullopt;
}

std::optional<std::string>
trigger_callbacks()
{
    log_i(mock_runtime, "Triggering callbacks");
    try {
        py::exec(R"(place_market_order("BUY", "A", 1.0, 1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run place_market_order: {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_orderbook_update("A","BUY",1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_orderbook_update: {}", e.what());
    }
    try {
        py::exec(R"(strategy.on_trade_and_account_update("A","BUY",1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_trade_and_account_update (trade): {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_trade_and_account_update("A","BUY",1.0,1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_trade_and_account_update (account): {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_holding_change())");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_holding_change: {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_orderbook_update("B","BUY",1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_orderbook_update: {}", e.what());
    }
    try {
        py::exec(R"(strategy.on_trade_and_account_update("B","BUY",1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_trade_and_account_update (trade): {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_trade_and_account_update("B","BUY",1.0,1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_trade_and_account_update (account): {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_holding_change())");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_holding_change: {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_orderbook_update("C","BUY",1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_orderbook_update: {}", e.what());
    }
    try {
        py::exec(R"(strategy.on_trade_and_account_update("C","BUY",1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_trade_and_account_update (trade): {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_trade_and_account_update("C","BUY",1.0,1.0,1.0))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_trade_and_account_update (account): {}", e.what());
    }

    try {
        py::exec(R"(strategy.on_holding_change())");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_holding_change: {}", e.what());
    }

    return std::nullopt;
}

} // namespace pywrapper
} // namespace nutc
