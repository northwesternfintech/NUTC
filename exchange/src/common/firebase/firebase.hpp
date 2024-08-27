#pragma once

#include <curl/curl.h>
#include <glaze/glaze.hpp>

#include <optional>
#include <string>

namespace nutc::common {

// database request - change name
glz::json_t firebase_request(
    const std::string& method, const std::string& url, const std::string& data = ""
);

glz::json_t get_user_info(const std::string& uid);

std::optional<std::string> get_algo(const std::string& uid, const std::string& algo_id);

} // namespace nutc::common
