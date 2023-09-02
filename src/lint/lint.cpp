#include "lint.hpp"

namespace nutc {
namespace lint {
std::string
lint(const std::string& uid, const std::string& algo_id)
{
    std::optional<std::string> algoCode = nutc::client::get_algo(uid, algo_id);
    if (!algoCode.has_value()) {
        return "Could not find algorithm";
    }

    pybind11::scoped_interpreter guard{};
    bool e = nutc::pywrapper::create_api_module(nutc::mock_api::getMarketFunc());
    if (!e) {
        log_e(linting, "Failed to create API module");
        nutc::client::set_lint_result(uid, algo_id, false);
        return "Unexpected error: failed to create API module";
    }

    std::optional<std::string> err = nutc::pywrapper::import_py_code(algoCode.value());
    if (err.has_value()) {
        log_e(linting, "{}", err.value());
        nutc::client::set_lint_result(uid, algo_id, false);
        nutc::client::set_lint_failure(uid, algo_id, err.value());
        return err.value();
    }

    err = nutc::pywrapper::run_initialization();
    if (err.has_value()) {
        log_e(linting, "{}", err.value());
        nutc::client::set_lint_result(uid, algo_id, false);
        nutc::client::set_lint_failure(uid, algo_id, err.value());
        return err.value();
    }

    err = nutc::pywrapper::trigger_callbacks();
    if (err.has_value()) {
        log_e(linting, "{}", err.value());
        nutc::client::set_lint_result(uid, algo_id, false);
        nutc::client::set_lint_failure(uid, algo_id, err.value());
        return err.value();
    }

    nutc::client::set_lint_result(uid, algo_id, true);

    return "Lint succeeded!";
}
} // namespace lint
} // namespace nutc
