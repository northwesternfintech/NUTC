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
    py::exec(R"(
        def publish_market_order(symbol, quantity, is_buy, client_order_id):
            nutc_api.publish_market_order(symbol, quantity, is_buy, client_order_id)
        print("Ready!")
        publish_market_order("ETHUSD",5,True,"test")
        print("Published!")
    )");
}

void
init(std::function<bool(const std::string&, int, bool, const std::string&)>
         publish_market_order)
{
    py::scoped_interpreter guard{};
    create_api_module(publish_market_order);
}

} // namespace pywrapper
} // namespace nutc
