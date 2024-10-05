#pragma once

#include <glaze/core/common.hpp>

#include <string>

namespace nutc {
namespace lint {

struct lint_result {
    bool success;
    std::string message;
};

} // namespace lint
} // namespace nutc

template <>
struct glz::meta<nutc::lint::lint_result> {
    using t = nutc::lint::lint_result;
    static constexpr auto value =
        object("success", &t::success, "message", &t::message);
};
