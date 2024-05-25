#include "lint.hpp"

#include "pywrapper/runtime.hpp"

#include <fmt/core.h>
#include <pybind11/pybind11.h>

#include <optional>
#include <string>

namespace nutc {
namespace lint {

namespace {
bool
mock_market_func(const std::string&, const std::string&, float, float)
{
    return true;
}
} // namespace

lint_result
lint(const std::string& algo_code)
{
    std::string out_message = "[linter] starting to lint algorithm\n";
    bool ok = nutc::pywrapper::create_api_module(mock_market_func);
    if (!ok) {
        out_message += "[linter] failed to create API module\n";
        return {false, out_message};
    }
    ok = nutc::pywrapper::supress_stdout();
    if (!ok) {
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
