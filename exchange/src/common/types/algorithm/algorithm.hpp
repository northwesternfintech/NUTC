#pragma once

#include "local_algorithm.hpp"
#include "remote_algorithm.hpp"

namespace nutc::common {
using algorithm_variant = std::variant<RemoteAlgorithm, LocalAlgorithm>;

inline std::string
get_id(const algorithm_variant& algo_variant)
{
    return std::visit([](const auto& algo) { return algo.get_id(); }, algo_variant);
}
} // namespace nutc::common
