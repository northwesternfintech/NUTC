#include "runtime.hpp"

namespace py = pybind11;

namespace nutc {
namespace pywrapper {

bool
create_api_module(std::function<bool(const std::string&, int, bool, const std::string&)>
                      publish_market_order)
{
    try {
        py::module m = py::module::create_extension_module(
            "nutc_api", "NUTC Exchange API", new py::module::module_def
        );
        m.def("publish_market_order", publish_market_order);

        py::module_ sys = py::module_::import("sys");
        py::dict sys_modules = sys.attr("modules").cast<py::dict>();
        sys_modules["nutc_api"] = m;

        py::exec(R"(import nutc_api)");
        py::exec(R"(
        def place_market_order(symbol, quantity, is_buy, client_order_id):
            nutc_api.publish_market_order(symbol, quantity, is_buy, client_order_id)
    )");
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
    try {
        py::object main_module = py::module_::import("__main__");
        py::dict main_dict = main_module.attr("__dict__");
        py::object on_order_update = main_dict["on_order_update"];
        py::object on_trade_update = main_dict["on_trade_update"];
        py::object on_orderbook_update = main_dict["on_orderbook_update"];
        py::object on_account_update = main_dict["on_account_update"];
    } catch (py::error_already_set& e) {
        return fmt::format("Failed to import callback functions: {}", e.what());
    }

    return std::nullopt;
}

std::optional<std::string>
run_initialization()
{
    log_i(mock_runtime, "Running initialization code");
    try {
        py::exec(R"(initialize())");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run initialization: {}", e.what());
    }
    return std::nullopt;
}

std::optional<std::string>
trigger_callbacks()
{
    log_i(mock_runtime, "Triggering callbacks");
    try {
        py::exec(R"(place_market_order("ETHUSD", 1, True, "market"))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run place_market_order: {}", e.what());
    }

    try {
        py::exec(R"(on_trade_update("ETHUSD",1,1))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_trade_update: {}", e.what());
    }
    try {
        py::exec(R"(on_orderbook_update("ETHUSD","L1",False))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_orderbook_update: {}", e.what());
    }

    try {
        py::exec(R"(on_account_update("ETH",1,1))");
    } catch (const std::exception& e) {
        return fmt::format("Failed to run on_account_update: {}", e.what());
    }
    return std::nullopt;
}

} // namespace pywrapper
} // namespace nutc
