#pragma once

#include <fmt/core.h>

#include <cstdlib>

#include <string>

namespace nutc::testing {
// TODO: filepaths
inline void
compile_cpp(const std::string& algo_name)
{
    static constexpr std::string_view TEMPLATE_PATH = "test_algos/cpp/template.cpp";
    std::string output_path = fmt::format("test_algos/cpp/{}.so", algo_name);
    std::string input_path = fmt::format("test_algos/cpp/{}.hpp", algo_name);
    std::string command = fmt::format(
        "g++ -std=c++23 -fPIC -shared -o {} -include {} {}", output_path, input_path,
        TEMPLATE_PATH
    );

    int result = system(command.c_str());

    if (result != 0) {
        throw std::runtime_error(fmt::format("Compilation of {} failed", input_path));
    }
}
} // namespace nutc::testing
