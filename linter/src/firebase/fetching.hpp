#pragma once

#include "config.h"
#include "logging.hpp"

#include <curl/curl.h>
#include <glaze/glaze.hpp>

#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <string>

namespace nutc {
namespace client {

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
std::optional<std::string> get_algo_status(
    const std::string& uid, const std::string& algo_id
); // failure, pending, success

} // namespace client
} // namespace nutc
