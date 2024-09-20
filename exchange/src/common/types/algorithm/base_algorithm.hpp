#pragma once

#include <fmt/format.h>

namespace nutc::common {
enum class AlgoLanguage { python, cpp };
enum class AlgoLocation { local, s3 };

class BaseAlgorithm {
    AlgoLanguage language_;

public:
    AlgoLanguage
    get_language() const
    {
        return language_;
    }

    explicit BaseAlgorithm(AlgoLanguage language) : language_{language} {}
};

} // namespace nutc::common
