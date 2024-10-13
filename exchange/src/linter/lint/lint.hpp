#pragma once

#include "linter/lint/lint_result.hpp"
#include "linter/runtime/runtime.hpp"

namespace nutc::linter {

[[nodiscard]] lint_result lint(Runtime& runtime);

} // namespace nutc::linter
