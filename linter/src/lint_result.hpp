#pragma once

#include <glaze/core/common.hpp>

#include <string>

struct lint_result {
    bool success;
    std::string message;
};

template <>
struct glz::meta<lint_result> {
    using t = lint_result;
    static constexpr auto value =
        object("success", &t::success, "message", &t::message);
};
