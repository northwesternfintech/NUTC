#pragma once

#include "lint_result.hpp"

#include <string>

namespace nutc {
namespace lint {

[[nodiscard]] lint_result lint(const std::string& algo_code);

} // namespace lint
} // namespace nutc
