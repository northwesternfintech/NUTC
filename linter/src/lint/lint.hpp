#pragma once

#include "lint/lint_result.hpp"
#include "runtime/runtime.hpp"

namespace nutc {
namespace lint {

[[nodiscard]] lint_result lint(Runtime& runtime);

} // namespace lint
} // namespace nutc
