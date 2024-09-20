#include "local_algorithm.hpp"

#include "base_algorithm.hpp"
#include "common/file_operations/file_operations.hpp"

#include <boost/filesystem.hpp>

#include <cassert>

#include <filesystem>

namespace nutc::common {
namespace {
std::string
get_cpp_template_path()
{
    static const char* template_path_env = std::getenv("NUTC_CPP_TEMPLATE_PATH");
    if (template_path_env == nullptr)
        throw std::runtime_error("Template.cpp path not set, unable to compile cpp");
    return template_path_env;
}
} // namespace

std::string
LocalAlgorithm::compile_cpp(const std::filesystem::path& filepath)
{
    std::string binary_output = (boost::filesystem::temp_directory_path()
                                 / boost::filesystem::unique_path("%%%%-%%%%-%%%%.tmp"))
                                    .string();

    std::string command = fmt::format(
        "g++ -std=c++20 -fPIC -shared -o {} -include {} {}", binary_output,
        filepath.string(), get_cpp_template_path()
    );

    int result = system(command.c_str());

    if (result != 0) {
        throw std::runtime_error(
            fmt::format("Compilation of {} failed", filepath.string())
        );
    }
    return binary_output;
}

LocalAlgorithm::LocalAlgorithm(AlgoLanguage language, std::filesystem::path filepath) :
    BaseAlgorithm{language}, filepath_{std::move(filepath)}
{}

const std::filesystem::path&
LocalAlgorithm::get_path() const
{
    return filepath_;
}

std::string
LocalAlgorithm::get_algo_string() const
{
    if (get_language() == AlgoLanguage::cpp) {
        return common::read_file_content(compile_cpp(filepath_));
    }
    if (get_language() == AlgoLanguage::python) {
        return common::read_file_content(filepath_);
    }

    throw std::runtime_error("Unknown algo language");
}

std::string
LocalAlgorithm::get_id() const
{
    return filepath_.string();
}
} // namespace nutc::common
