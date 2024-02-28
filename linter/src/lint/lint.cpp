#include "lint.hpp"

namespace nutc {
namespace lint {
bool
lint(
    const std::string& uid, const std::string& algo_id, std::stringstream& output_stream
)
{
    std::optional<std::string> algoCode = nutc::client::get_algo(uid, algo_id);
    if (!algoCode.has_value()) {
        output_stream << "[linter] FAILURE - could not find algo id " << algo_id
                      << " for uid " << uid << "\n";
        nutc::client::set_lint_failure(uid, algo_id, "Could not find algo id");
        return false;
    }

    bool e = nutc::pywrapper::create_api_module(nutc::mock_api::getMarketFunc());
    if (!e) {
        log_e(linting, "Failed to create API module");
        output_stream << "[linter] failed to create API module\n";
        nutc::client::set_lint_failure(uid, algo_id, "Failed to create API module");
        return false;
    }

    std::optional<std::string> err = nutc::pywrapper::import_py_code(algoCode.value());
    if (err.has_value()) {
        log_e(linting, "{}", err.value());
        output_stream << err.value() << "\n";
        nutc::client::set_lint_failure(uid, algo_id, err.value());
        return false;
    }

    err = nutc::pywrapper::run_initialization();
    if (err.has_value()) {
        log_e(linting, "{}", err.value());
        output_stream << err.value() << "\n";
        nutc::client::set_lint_failure(uid, algo_id, err.value());
        return false;
    }

    err = nutc::pywrapper::trigger_callbacks();
    if (err.has_value()) {
        log_e(linting, "{}", err.value());
        output_stream << err.value() << "\n";
        nutc::client::set_lint_failure(uid, algo_id, err.value());
        return false;
    }

    output_stream << "[linter] linting process done!"
                  << "\n";

    nutc::client::set_lint_success(uid, algo_id, output_stream.str());

    return true;
}
} // namespace lint
} // namespace nutc
