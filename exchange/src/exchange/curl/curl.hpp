#pragma once

#include <glaze/glaze.hpp>

#include <string>

namespace nutc::exchange {
std::string request_to_string(
    const std::string& method, const std::string& url, const std::string& data = ""
);

void request_to_file(
    const std::string& method, const std::string& url, const std::string& filepath,
    const std::string& data = ""
);

/**
 * @brief Sends a request to the firebase server
 * @param method The HTTP method to use
 * @param url The URL to send the request to (including query)
 * @param data The string to write received data to
 * @return The response from the server in JSON format
 */
glz::json_t request_to_json(
    const std::string& method, const std::string& url, const std::string& data = ""
);

} // namespace nutc::exchange
