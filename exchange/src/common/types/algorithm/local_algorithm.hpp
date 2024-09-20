#pragma once
#include "base_algorithm.hpp"

#include <filesystem>

namespace nutc::common {
class LocalAlgorithm : public BaseAlgorithm {
    std::filesystem::path filepath_;

public:
    LocalAlgorithm(AlgoLanguage language, std::filesystem::path filepath);

    const std::filesystem::path& get_path() const;

    std::string get_algo_string() const;

    std::string get_id() const;

private:
    std::string compile_cpp_() const;
};
} // namespace nutc::common
