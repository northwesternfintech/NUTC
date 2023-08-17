#pragma once

#include <curl/curl.h>

#include <iostream>
#include <string>

namespace nutc {
namespace client {

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
std::string firebase_request(
    const std::string& method, const std::string& url, const std::string& data = ""
);

} // namespace client
} // namespace nutc
