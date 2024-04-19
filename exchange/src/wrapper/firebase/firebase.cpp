#include "firebase.hpp"

#include "wrapper/config.h"

#include <fmt/format.h>

namespace nutc {
namespace firebase {

glz::json_t
get_user_info(const std::string& uid)
{
    auto url = fmt::format("{}/users/{}.json", std::string(FIREBASE_URL), uid);
    return firebase_request("GET", url);
}

namespace {
size_t
write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
    auto* str = static_cast<std::string*>(userp);
    auto* data = static_cast<char*>(contents);

    str->append(data, size * nmemb);
    return size * nmemb;
}
} // namespace

std::string
storage_request(const std::string& firebase_url)
{
    CURL* curl{};
    CURLcode res{};
    std::string read_buffer{};

    curl = curl_easy_init();
    if (curl == nullptr)
        throw std::runtime_error("curl_easy_init() failed");

    curl_easy_setopt(curl, CURLOPT_URL, firebase_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(
            fmt::format("curl_easy_perform() failed: {}", curl_easy_strerror(res))
        );
    }

    curl_easy_cleanup(curl);

    return read_buffer;
}

std::optional<std::string>
get_algo(const std::string& uid, const std::string& algo_id)
{
    glz::json_t user_info = get_user_info(uid);
    if (!user_info.contains("algos"))
        return std::nullopt;

    glz::json_t algos = user_info["algos"];
    if (!algos.contains(algo_id))
        return std::nullopt;

    glz::json_t algo_info = algos[algo_id];
    if (!algo_info.contains("downloadURL"))
        return std::nullopt;

    std::string download_url = algo_info["downloadURL"].get<std::string>();
    std::string algo_file = storage_request(download_url);
    return algo_file;
}

glz::json_t
firebase_request(
    const std::string& method, const std::string& url, const std::string& data
)
{
    CURL* curl{};
    CURLcode res{};
    std::string read_buffer{};

    curl = curl_easy_init();
    if (curl == nullptr)
        throw std::runtime_error("curl_easy_init() failed");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    }
    else if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    }
    else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        throw std::runtime_error(
            fmt::format("curl_easy_perform() failed: {}", curl_easy_strerror(res))
        );
    }

    curl_easy_cleanup(curl);

    glz::json_t json{};
    auto error = glz::read_json(json, read_buffer);
    if (error) {
        std::string descriptive_error = glz::format_error(error, read_buffer);
        throw std::runtime_error(
            fmt::format("glz::read_json() failed: {}", descriptive_error)
        );
    }
    return json;
}
} // namespace firebase
} // namespace nutc
