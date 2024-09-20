#include "local_algorithm.hpp"

#include "base_algorithm.hpp"
#include "common/file_operations/file_operations.hpp"

#include <cassert>

#include <optional>

namespace nutc::common {
std::string
LocalAlgorithm::compile_cpp_() const
{
    assert(get_language() == AlgoLanguage::cpp);
    static constexpr std::string_view TEMPLATE_PATH = "test_algos/cpp/template.cpp";
    std::string output_path = fmt::format("/tmp/algo_cpp_path.so", filepath_.string());
    std::string command = fmt::format(
        "g++ -std=c++20 -fPIC -shared -o {} -include {} {}", output_path,
        filepath_.string(), TEMPLATE_PATH
    );

    int result = system(command.c_str());

    if (result != 0) {
        throw std::runtime_error(
            fmt::format("Compilation of {} failed", filepath_.string())
        );
    }
    return output_path;
}

LocalAlgorithm::LocalAlgorithm(AlgoLanguage language, std::filesystem::path filepath) :
    BaseAlgorithm{language}, filepath_{std::move(filepath)}
{
    if (!std::filesystem::exists(filepath_)) [[unlikely]] {
        throw std::runtime_error(
            fmt::format("Local algorithm file not found: {}", filepath_.string())
        );
    }
}

const std::filesystem::path&
LocalAlgorithm::get_path() const
{
    return filepath_;
}

std::string
LocalAlgorithm::get_algo_string() const
{
    std::optional<std::string> algorithm;
    if (get_language() == AlgoLanguage::cpp) {
        algorithm = common::read_file_content(compile_cpp_());
    }
    else {
        algorithm = common::read_file_content(filepath_);
    }

    if (!algorithm) {
        throw std::runtime_error(
            fmt::format("Unable to find algorithm at {}", filepath_.string())
        );
    }

    return algorithm.value();
}

std::string
LocalAlgorithm::get_id() const
{
    return filepath_.string();
}
} // namespace nutc::common
