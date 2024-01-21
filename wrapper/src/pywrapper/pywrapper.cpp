#include "pywrapper.hpp"

namespace nutc {
namespace pywrapper {

void
create_api_module(
    std::function<bool(const std::string&, const std::string&, float, float)>
        publish_market_order
)
{
    py::module m = py::module::create_extension_module(
        "nutc_api", "NUTC Exchange API", new py::module::module_def
    );
    m.def("publish_market_order", publish_market_order);

    py::module_ sys = py::module_::import("sys");
    py::dict sys_modules = sys.attr("modules").cast<py::dict>();
    sys_modules["nutc_api"] = m;

    py::exec(R"(import nutc_api)");
}

py::object
get_ob_update_function()
{
    return py::globals()["strategy"].attr("on_orderbook_update");
}

py::object
get_trade_and_account_update_function()
{
    return py::globals()["strategy"].attr("on_trade_and_account_update");
}

// A tiny amount of backwards compatability
py::object
get_trade_update_function()
{
    return py::globals()["strategy"].attr("on_trade_and_account_update");
}

py::object
get_account_update_function()
{
    return py::globals()["strategy"].attr("on_trade_and_account_update");
}

void
run_code_init(const std::string& py_code)
{
    py::exec(py_code);
    py::exec(R"(
        def place_market_order(side, ticker, quantity, price):
            nutc_api.publish_market_order(side, ticker, quantity, price)
    )");
    py::exec("strategy = Strategy()");
}

} // namespace pywrapper
} // namespace nutc
