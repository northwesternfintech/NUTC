#include "fetching.hpp"

#include "config.h"
#include "config.h.in"

#include <curl/curl.h>
#include <fmt/core.h>

#include <iostream>
#include <optional>
#include <regex>

namespace {
std::string
replaceDisallowedValues(const std::string& input)
{
    std::regex newlinePattern("\\n");
    std::string input2 = std::regex_replace(input, newlinePattern, "\\n");
    std::regex disallowedPattern("[.$#\\[\\]]");

    return std::regex_replace(input2, disallowedPattern, "");
}
} // namespace

namespace nutc {
namespace client {

void
set_lint_result(
    const std::string& uid,
    const std::string& algo_id,
    bool succeeded,
    const std::string& message
)
{
    std::string endpoint =
        fmt::format("{}/lint-result/{}/{}", WEBSERVER_URL, uid, algo_id);

    SetLintBody body{succeeded, replaceDisallowedValues(message)};

    database_request("POST", endpoint, glz::write_json(body), true);
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

std::optional<std::string>
get_algo(const std::string& algo_id)
{
    std::string url = fmt::format("{}/{}/{}", S3_URL, S3_BUCKET, algo_id);
    std::cout << "requesting file from [" << url << "]\n";
    auto algo_file = storage_request(url);
    return algo_file;
}

std::optional<glz::json_t>
database_request(
    const std::string& method,
    const std::string& url,
    const std::string& data,
    bool json_body
)
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

        if (json_body) {
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            if (res != CURLE_OK) {
                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
                return std::nullopt;
            }
        }

        if (method == "POST") {
            res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            if (res != CURLE_OK) {
                curl_easy_cleanup(curl);
                return std::nullopt;
            }
        }
        else if (method == "PUT") {
            res = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            if (res != CURLE_OK) {
                return std::nullopt;
            }
            res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            if (res != CURLE_OK) {
                return std::nullopt;
            }
        }
        else if (method == "DELETE") {
            res = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            if (res != CURLE_OK) {
                return std::nullopt;
            }
        }

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            return std::nullopt;
        }

        curl_easy_cleanup(curl);
    }
    glz::json_t json{};
    auto error = glz::read_json(json, readBuffer);
    if (error) {
        return std::nullopt;
    }
    return json;
}
} // namespace client
} // namespace nutc
