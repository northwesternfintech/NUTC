#pragma once

#include "config.h"
#include "logging.hpp"

#include <curl/curl.h>
#include <glaze/glaze.hpp>

#include <iostream>
#include <map>
#include <optional>
#include <string>

namespace nutc {
namespace firebase {

// database request - change name
glz::json_t firebase_request(
    const std::string& method, const std::string& url, const std::string& data = ""
);

std::string storage_request(const std::string& id, const std::string& url);

glz::json_t get_user_info(const std::string& uid);

std::optional<std::string> get_algo(const std::string& uid, const std::string& algo_id);

} // namespace firebase
} // namespace nutc
