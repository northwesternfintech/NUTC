#pragma once

#include "linter/lint/lint_result.hpp"
#include "linter/runtime/runtime.hpp"

namespace nutc {
namespace lint {

[[nodiscard]] lint_result lint(Runtime& runtime);

} // namespace lint
} // namespace nutc
