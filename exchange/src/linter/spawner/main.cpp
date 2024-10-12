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

void
send_response(const std::string& response)
{
    std::cout << nutc::common::base64_encode(response) << std::endl;
}

int
main(int argc, char* argv[])
{
    using namespace nutc::common;
    using namespace nutc::linter;

    if (argc < 2) {
        send_response("[linter] no language provided");
        return 1;
    }

    std::string algo_code_base64;
    std::getline(std::cin, algo_code_base64);

    std::string algo_code = nutc::common::base64_decode(algo_code_base64);

    lint_result lint_result;
    std::string flag = argv[1];
    if (flag == "-python") {
        PyRuntime runtime(
            algo_code, mock_limit_func, mock_market_func, mock_cancel_func
        );
        lint_result = lint(runtime);
    }
    else if (flag == "-cpp") {
        CppRuntime runtime(
            algo_code, mock_limit_func, mock_market_func, mock_cancel_func
        );
        lint_result = lint(runtime);
    }
    else {
        send_response("[linter] no language provided");
        return 1;
    }

    auto output = glz::write_json(lint_result);
    if (output) {
        send_response(output.value());
    }
    else {
        send_response(fmt::format(
            "[linter] ERROR WRITING LINT RESULT: {}", glz::format_error(output.error())
        ));
    }

    return 0;
}
