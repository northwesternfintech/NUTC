#include "pywrapper.hpp"

namespace nutc {
namespace pywrapper {

void
create_api_module(
    std::function<bool(const std::string&, const std::string&, double, double)>
        publish_limit_order,
    std::function<bool(const std::string&, const std::string&, double)>
        publish_market_order
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

py::object
get_ob_update_function()
{
    return py::globals()["strategy"].attr("on_orderbook_update");
}

py::object
get_trade_update_function()
{
    return py::globals()["strategy"].attr("on_trade_update");
}

py::object
get_account_update_function()
{
    return py::globals()["strategy"].attr("on_account_update");
}

void
run_code_init(const std::string& py_code)
{
    py::exec(py_code);
    py::exec(R"(
        def place_market_order(side, ticker, quantity):
            return nutc_api.publish_market_order(side, ticker, quantity)
    )");
    py::exec(R"(
        def place_limit_order(side, ticker, price, quantity):
            return nutc_api.publish_limit_order(side, ticker, price, quantity)
    )");
    py::exec("strategy = Strategy()");
}

} // namespace pywrapper
} // namespace nutc
