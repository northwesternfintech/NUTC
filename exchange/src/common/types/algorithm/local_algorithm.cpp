#include "local_algorithm.hpp"

#include "common/file_operations/file_operations.hpp"

#include <optional>

namespace nutc::common {
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
    std::optional<std::string> algorithm = nutc::common::read_file_content(filepath_);

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
