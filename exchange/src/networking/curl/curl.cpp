#include "curl.hpp"

#include "logging.hpp"

#include <curl/curl.h>

namespace nutc {
namespace curl {

static size_t
write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
    auto* str = reinterpret_cast<std::string*>(userp);
    auto* data = static_cast<char*>(contents);

    str->append(data, size * nmemb);
    return size * nmemb;
}

static size_t
file_write_callback(void* contents, size_t size, size_t nmemb, FILE* stream)
{
    return fwrite(contents, size, nmemb, stream);
}

void
request_to_file(
    const std::string& method, const std::string& url, const std::string& filepath,
    const std::string& data
)
{
    CURL* curl;
    CURLcode res;
    FILE* fp;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(filepath.c_str(), "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, file_write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

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
            log_e(
                firebase_fetching, "curl_easy_perform() failed: {}",
                curl_easy_strerror(res)
            );
        }

        curl_easy_cleanup(curl);
    }
}

std::string
request_to_string(
    const std::string& method, const std::string& url, const std::string& data
)
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

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
            log_e(
                firebase_fetching, "curl_easy_perform() failed: {}",
                curl_easy_strerror(res)
            );
        }

        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

glz::json_t
request_to_json(
    const std::string& method, const std::string& url, const std::string& data
)
{
    std::string readBuffer = request_to_string(method, url, data);
    glz::json_t json{};
    auto error = glz::read_json(json, readBuffer);
    if (error) {
        std::string descriptive_error = glz::format_error(error, readBuffer);
        log_e(firebase_fetching, "glz::read_json() failed: {}", descriptive_error);
    }
    return json;
}
} // namespace curl
} // namespace nutc
