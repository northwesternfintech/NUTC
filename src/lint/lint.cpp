#include "lint.hpp"

namespace nutc {
namespace lint {
std::string
lint(const std::string& uid, const std::string& algo_id)
{
    std::optional<std::string> algoCode = nutc::client::get_most_recent_algo(uid);
    if (!algoCode.has_value()) {
        return 0;
    }

    pybind11::scoped_interpreter guard{};
    bool e = nutc::pywrapper::create_api_module(nutc::mock_api::getMarketFunc());
    if (!e) {
        log_e(linting, "Failed to create API module");
        return "Unexpected error: failed to create API module";
    }

    std::optional<std::string> err = nutc::pywrapper::import_py_code(algoCode.value());
    if (err.has_value()) {
        log_e(linting, "{}", err.value());
        return err.value();
    }

    err = nutc::pywrapper::run_initialization();
    if (err.has_value()) {
        log_e(linting, "{}", err.value());
        return err.value();
    }

    err = nutc::pywrapper::trigger_callbacks();
    if (err.has_value()) {
        log_e(linting, "{}", err.value());
        return err.value();
    }

    return "Lint succeeded!";
}
} // namespace lint
} // namespace nutc
