#include "fetching.hpp"

#include <curl/curl.h>
#include <fmt/core.h>

#include <optional>
#include <regex>

namespace nutc {
namespace client {
std::string
replaceDisallowedValues(const std::string& input)
{
    std::regex newlinePattern("\\n");
    std::string input2 = std::regex_replace(input, newlinePattern, "\\n");
    std::regex disallowedPattern("[.$#\\[\\]]");

    return std::regex_replace(input2, disallowedPattern, "");
}

static size_t
write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
    auto* str = reinterpret_cast<std::string*>(userp);
    auto* data = static_cast<char*>(contents);

    str->append(data, size * nmemb);
    return size * nmemb;
}

std::optional<std::string>
storage_request(const std::string& url)
{
    std::string readBuffer;

    CURL* curl = curl_easy_init();
    if (curl) {
        CURLcode res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if (res != CURLE_OK) {
            return std::nullopt;
        }
        res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        if (res != CURLE_OK) {
            return std::nullopt;
        }
        res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        if (res != CURLE_OK) {
            return std::nullopt;
        }

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            return std::nullopt;
        }
        curl_easy_cleanup(curl);
    }

    return readBuffer;
}

} // namespace client
} // namespace nutc
