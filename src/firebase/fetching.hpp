#pragma once

#include "config.h"
#include "logging.hpp"

#include <curl/curl.h>
#include <glaze/glaze.hpp>

#include <iostream>
#include <map>
#include <string>
#include <optional>

namespace nutc {
namespace client {

//database request - change name
glz::json_t firebase_request(
    const std::string& method, const std::string& url, const std::string& data = ""
);

std::string storage_request(const std::string& url);

glz::json_t get_user_info(const std::string& uid);

void set_lint_result(const std::string& uid, const std::string& algo_id, bool succeeded);

std::optional<std::string> get_algo(const std::string& uid, const std::string& algo_id);

} // namespace client
} // namespace nutc
