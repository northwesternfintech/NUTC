#pragma once

#include "config.h"
#include "logging.hpp"

#include <curl/curl.h>
#include <glaze/glaze.hpp>

#include <iostream>
#include <map>
#include <string>

namespace nutc {
namespace client {

glz::json_t firebase_request(
    const std::string& method, const std::string& url, const std::string& data = ""
);

glz::json_t get_user_info(const std::string& uid);

} // namespace client
} // namespace nutc
