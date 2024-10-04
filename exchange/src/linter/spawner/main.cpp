#include "common/types/decimal.hpp"
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
mock_limit_func(nutc::common::Side, nutc::common::Ticker, float, float, bool = false)
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

    std::cerr << "TESTING" << std::endl;

    std::string algo_code_base64;
    std::getline(std::cin, algo_code_base64);
    std::cerr << "gotline" << std::endl;

    std::string algo_code = nutc::common::base64_decode(algo_code_base64);
    std::cerr << "decoded\n" << algo_code << std::endl;

    nutc::lint::lint_result lint_result;
    std::string flag = argv[1];
    if (flag == "-python") {
        nutc::lint::PyRuntime runtime(
            algo_code, mock_limit_func, mock_market_func, mock_cancel_func
        );
        lint_result = nutc::lint::lint(runtime);
    }
    else if (flag == "-cpp") {
        std::cerr << "cpprun" << std::endl;
        nutc::lint::CppRuntime runtime(
            algo_code, mock_limit_func, mock_market_func, mock_cancel_func
        );
        std::cerr << "cpplint" << std::endl;
        lint_result = nutc::lint::lint(runtime);
        std::cerr << "lint output" << std::endl << lint_result.message << std::endl;
        std::cerr << "cpplintdone" << std::endl;
    }
    else {
        std::cout << "[linter] no language provided\n";
        return 1;
    }

    auto output = glz::write_json(lint_result);
    if (output) {
        std::cerr << "output" << *output << std::endl;
        std::cout << *output << std::endl;
    }
    else {
        std::cerr << "error" << std::endl;
        std::cout << fmt::format(
            "[linter] ERROR WRITING LINT RESULT: {}", glz::format_error(output.error())
        ) << std::endl;
    }

    return 0;
}
