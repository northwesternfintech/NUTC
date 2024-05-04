#include "lint.hpp"

#include "mock_api/mock_api.hpp"
#include "pywrapper/runtime.hpp"

#include <fmt/core.h>
#include <pybind11/pybind11.h>

#include <optional>
#include <string>

namespace nutc {
namespace lint {

lint_result
lint(const std::string& algo_code)
{
    std::string out_message = "[linter] starting to lint algorithm\n";
    bool e = nutc::pywrapper::create_api_module(nutc::mock_api::getMarketFunc());
    if (!e) {
        out_message += "[linter] failed to create API module\n";
        return {false, out_message};
    }
    e = nutc::pywrapper::supress_stdout();
    if (!e) {
        out_message += "[linter] failed to initialize python environment\n";
        return {false, out_message};
    }

    std::optional<std::string> err = nutc::pywrapper::import_py_code(algo_code);
    if (err.has_value()) {
        out_message += fmt::format("{}\n", err.value());
        return {false, out_message};
    }

    err = nutc::pywrapper::run_initialization();
    if (err.has_value()) {
        out_message += fmt::format("{}\n", err.value());
        return {false, out_message};
    }

    err = nutc::pywrapper::trigger_callbacks();
    if (err.has_value()) {
        out_message += fmt::format("{}\n", err.value());
        return {false, out_message};
    }
    out_message += "\n[linter] linting process succeeded!\n";
    return {true, out_message};
}

} // namespace lint
} // namespace nutc
