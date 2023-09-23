#pragma once

#include <curl/curl.h>
#include <glaze/glaze.hpp>

#include <iostream>
#include <map>
#include <string>

namespace nutc {
/**
 * @brief Handles all communication with firebase
 */
namespace firebase {

/**
 * @brief Sends a request to the firebase server
 * @param method The HTTP method to use
 * @param url The URL to send the request to (including query)
 * @param data The string to write received data to
 * @return The response from the server in JSON format
 */
glz::json_t firebase_request(
    const std::string& method, const std::string& url, const std::string& data = ""
);

} // namespace firebase
} // namespace nutc
