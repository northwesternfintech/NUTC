#pragma once

#include <fmt/core.h>

#include <cstdlib>

#include <print>
#include <string>

namespace nutc::testing {
// TODO: filepaths
inline void
compile_cpp(
    const std::string& input_path, const std::string& output_path,
    const std::string& template_path = "test_algos/cpp/template.cpp"
)
{
    std::string command = fmt::format(
        "g++ -std=c++20 -fPIC -shared -o {} -include {} {}", output_path, input_path,
        template_path

    );
    std::println("{}", command);

    int result = system(command.c_str());

    if (result != 0) {
        throw std::runtime_error(fmt::format("Compilation of {} failed", input_path));
    }
}
} // namespace nutc::testing
