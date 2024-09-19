#pragma once

#include "common/file_operations/file_operations.hpp"
#include <fmt/format.h>

#include <filesystem>
#include <optional>
#include <string>
#include <variant>

namespace nutc::common {
enum class AlgoLanguage { python, cpp };
enum class AlgoLocation { local, s3 };

class Algorithm {
    AlgoLanguage language_;

public:
    AlgoLanguage
    get_language() const
    {
        return language_;
    }

    explicit Algorithm(AlgoLanguage language) : language_{language} {}
};

class RemoteAlgorithm : public Algorithm {
public:
    // TODO
    RemoteAlgorithm() : Algorithm(AlgoLanguage::cpp)
    {
        throw std::runtime_error("Not implemented");
    }

    static std::string
    get_algo_string()
    {
        throw std::runtime_error("Not implemented");
    }

    // todo: deprecate
    static std::string
    get_id()
    {
        throw std::runtime_error("Not implemented");
    }
};

class LocalAlgorithm : public Algorithm {
    std::filesystem::path filepath_;

public:
    LocalAlgorithm(AlgoLanguage language, std::filesystem::path filepath) :
        Algorithm{language}, filepath_{std::move(filepath)}
    {
        if (!std::filesystem::exists(filepath_)) [[unlikely]] {
            throw std::runtime_error(
                fmt::format("Local algorithm file not found: {}", filepath_.string())
            );
        }
    }

    const std::filesystem::path&
    get_path() const
    {
        return filepath_;
    }

    std::string
    get_algo_string() const
    {
        std::optional<std::string> algorithm =
            nutc::common::read_file_content(filepath_);

        if (!algorithm) {
            throw std::runtime_error(
                fmt::format("Unable to find algorithm at {}", filepath_.string())
            );
        }

        return algorithm.value();
    }

    std::string
    get_id() const
    {
        return filepath_.string();
    }
};

using algorithm_variant = std::variant<RemoteAlgorithm, LocalAlgorithm>;

inline std::string
get_id(const algorithm_variant& algo_variant)
{
    return std::visit([](const auto& algo) { return algo.get_id(); }, algo_variant);
}
} // namespace nutc::common
