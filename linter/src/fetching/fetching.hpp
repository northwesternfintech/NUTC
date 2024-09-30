#pragma once

#include <curl/curl.h>
#include <glaze/glaze.hpp>

#include <optional>
#include <string>

namespace nutc {
namespace client {

struct SetLintBody {
    bool success;
    std::string message;
};

enum class LintingResultOption { UNKNOWN = -1, FAILURE, SUCCESS, PENDING };

std::optional<glz::json_t> database_request(
    const std::string& method,
    const std::string& url,
    const std::string& data = "",
    bool json_body = false
);

std::optional<std::string> storage_request(const std::string& url);

void set_lint_result(
    const std::string& uid,
    const std::string& algo_id,
    bool succeeded,
    const std::string& message
);

[[nodiscard]] std::optional<std::string> get_algo(const std::string& algo_id);

} // namespace client
} // namespace nutc

template <>
struct glz::meta<nutc::client::SetLintBody> {
    using t = nutc::client::SetLintBody;
    static constexpr auto value =
        object("success", &t::success, "message", &t::message);
};
