#pragma once

#include <glaze/core/common.hpp>

#include <string>

namespace nutc::linter {

struct lint_result {
    bool success;
    std::string message;
};

} // namespace nutc::linter

template <>
struct glz::meta<nutc::linter::lint_result> {
    using t = nutc::linter::lint_result;
    static constexpr auto value =
        object("success", &t::success, "message", &t::message);
};
