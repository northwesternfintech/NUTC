#include "lint.hpp"

namespace nutc {
namespace lint {
std::string
lint(
    const std::string& uid, const std::string& algo_id, std::stringstream& output_stream
)
{
    std::optional<std::string> algoCode = nutc::client::get_algo(uid, algo_id);
    if (!algoCode.has_value()) {
        output_stream << "[linter] FAILURE - could not find algo id " << algo_id
                      << " for uid " << uid << "<br/>";
        return "Could not find algorithm";
    }

    bool e = nutc::pywrapper::create_api_module(nutc::mock_api::getMarketFunc());
    if (!e) {
        log_e(linting, "Failed to create API module");
        output_stream << "[linter] failed to create API module <br/>";
        nutc::client::set_lint_result(uid, algo_id, false);
        return "Unexpected error: failed to create API module";
    }

    std::optional<std::string> err = nutc::pywrapper::import_py_code(algoCode.value());
    if (err.has_value()) {
        log_e(linting, "{}", err.value());
        output_stream << err.value() << "<br/>";
        nutc::client::set_lint_result(uid, algo_id, false);
        nutc::client::set_lint_failure(uid, algo_id, err.value());
        return err.value();
    }

    err = nutc::pywrapper::run_initialization();
    if (err.has_value()) {
        log_e(linting, "{}", err.value());
        output_stream << err.value() << "<br/>";
        nutc::client::set_lint_result(uid, algo_id, false);
        nutc::client::set_lint_failure(uid, algo_id, err.value());
        return err.value();
    }

    err = nutc::pywrapper::trigger_callbacks();
    if (err.has_value()) {
        log_e(linting, "{}", err.value());
        output_stream << err.value() << "<br/>";
        nutc::client::set_lint_result(uid, algo_id, false);
        nutc::client::set_lint_failure(uid, algo_id, err.value());
        return err.value();
    }

    nutc::client::set_lint_result(uid, algo_id, true);

    output_stream << "[linter] linting process done!"
                  << "<br/>";

    return "Lint succeeded!";
}
} // namespace lint
} // namespace nutc
