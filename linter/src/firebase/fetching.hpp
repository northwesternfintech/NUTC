#pragma once

#include <curl/curl.h>
#include <glaze/glaze.hpp>

#include <optional>
#include <string>

namespace nutc {
namespace client {

enum class LintingResultOption { UNKNOWN = -1, FAILURE, SUCCESS, PENDING };

// database request - change name
glz::json_t firebase_request(
    const std::string& method, const std::string& url, const std::string& data = ""
);

std::string storage_request(const std::string& url);

glz::json_t get_user_info(const std::string& uid);

void
set_lint_result(const std::string& uid, const std::string& algo_id, bool succeeded);
void set_lint_failure(
    const std::string& uid, const std::string& algo_id, const std::string& failure
);
void set_lint_success(
    const std::string& uid, const std::string& algo_id, const std::string& success
);

std::optional<std::string> get_algo(const std::string& uid, const std::string& algo_id);
LintingResultOption get_algo_status(
    const std::string& uid, const std::string& algo_id
); // returns a LintingResultOption enum

} // namespace client
} // namespace nutc
