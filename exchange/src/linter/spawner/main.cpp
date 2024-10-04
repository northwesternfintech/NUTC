#include "common/types/ticker.hpp"
#include "common/util.hpp"
#include "linter/lint/lint.hpp"
#include "linter/lint/lint_result.hpp"
#include "linter/runtime/cpp/cpp_runtime.hpp"
#include "linter/runtime/python/python_runtime.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <glaze/core/read.hpp>
#include <glaze/json/write.hpp>
#include <pybind11/embed.h>

#include <cstdlib>

#include <iostream>
#include <string>

namespace {
bool
mock_limit_func(nutc::common::Side, nutc::common::Ticker, float, float, bool)
{
    return true;
}

bool
mock_market_func(nutc::common::Side, nutc::common::Ticker, float)
{
    return true;
}

bool
mock_cancel_func(nutc::common::Ticker, std::int64_t)
{
    return true;
}
} // namespace

int
main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "[linter] no language provided\n";
        return 1;
    }

    std::string algo_code;
    std::string line;
    while (std::getline(std::cin, line)) {
        algo_code += line + '\n';
    }

    nutc::lint::lint_result lint_result;
    std::string flag = argv[1];
    if (flag == "-python") {
        nutc::lint::PyRuntime runtime(
            algo_code, mock_limit_func, mock_market_func, mock_cancel_func
        );
        lint_result = nutc::lint::lint(runtime);
    }
    else if (flag == "-cpp") {
        nutc::lint::CppRuntime runtime(
            algo_code, mock_limit_func, mock_market_func, mock_cancel_func
        );
        lint_result = nutc::lint::lint(runtime);
    }
    else {
        std::cout << "[linter] no language provided\n";
        return 1;
    }

    std::cout << *glz::write_json(lint_result) << "\n";

    return 0;
}
