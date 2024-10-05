#include "local_algorithm.hpp"

#include "base_algorithm.hpp"
#include "common/compilation/compile_cpp.hpp"
#include "common/file_operations/file_operations.hpp"

#include <boost/filesystem.hpp>

#include <cassert>

#include <filesystem>

namespace nutc::common {

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
        return common::read_file_content(common::compile_cpp(filepath_));
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
