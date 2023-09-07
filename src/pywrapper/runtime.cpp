#include "runtime.hpp"

namespace py = pybind11;

namespace nutc {
namespace pywrapper {

void
create_api_module(std::function<bool(const std::string&, int, bool, const std::string&)>
                      publish_market_order)
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

void
run_code(const std::string& py_code)
{
    log_i(py_runtime, "Running code:\n{}", py_code);
    py::exec(py_code);
}

void
init(std::function<bool(const std::string&, int, bool, const std::string&)>
         publish_market_order)
{
    create_api_module(publish_market_order);
}

} // namespace pywrapper
} // namespace nutc
