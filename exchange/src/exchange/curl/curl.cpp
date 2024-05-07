#include "curl.hpp"

#include <curl/curl.h>
#include <fmt/format.h>

#include <regex>

namespace nutc {
namespace curl {

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
upload_file(const std::string& filepath) // NOLINT
{
    std::string read_buffer{};

    auto* curl = curl_easy_init();
    curl_mime* form = nullptr;
    curl_mimepart* field = nullptr;

    // Create the form
    form = curl_mime_init(curl);

    // Fill in the file upload field
    field = curl_mime_addpart(form);
    curl_mime_name(field, "file");
    curl_mime_filedata(field, filepath.c_str());

    // Set the URL
    curl_easy_setopt(curl, CURLOPT_URL, "https://0x0.st");

    // Set the form post data
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

    // Set the callback function to receive the response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

    // Perform the request, res will get the return code
    auto res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        throw std::runtime_error("Res failed");
    }
    curl_mime_free(form);
    curl_easy_cleanup(curl);

    return read_buffer;
}

void
request_to_file(
    const std::string& method, const std::string& url, const std::string& filepath,
    const std::string& data
)
{
    CURL* curl = curl_easy_init();

    if (curl == nullptr)
        throw std::runtime_error("Failed to init curl");

    FILE* into_file = fopen(filepath.c_str(), "wb");
    if (into_file == nullptr) {
        throw std::runtime_error(fmt::format("failed to open file: {}", filepath));
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, into_file);

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

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        throw std::runtime_error(
            fmt::format("curl_easy_perform() failed: {}", curl_easy_strerror(res))
        );
    }

    curl_easy_cleanup(curl);
    int err = fclose(into_file);
    if (err != 0)
        throw std::runtime_error("Failed to close filestream");
}

namespace {
std::string
replace_disallowed(const std::string& input)
{
    std::regex newlinePattern("\\n");
    std::string input2 = std::regex_replace(input, newlinePattern, "\\n");
    std::regex disallowedPattern("[.$#\\[\\]]");

    return std::regex_replace(input2, disallowedPattern, "");
}
} // namespace

std::string
request_to_string(
    const std::string& method, const std::string& url, const std::string& data
)
{
    auto fixed_data = "\"" + replace_disallowed(data) + "\"";
    CURL* curl = curl_easy_init();
    std::string read_buffer{};

    if (curl == nullptr)
        throw std::runtime_error("Failed to initialize curl");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    }
    else if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fixed_data.c_str());
    }
    else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        throw std::runtime_error(
            fmt::format("curl_easy_perform() failed: {}", curl_easy_strerror(res))
        );
    }

    curl_easy_cleanup(curl);
    return read_buffer;
}

glz::json_t
request_to_json(
    const std::string& method, const std::string& url, const std::string& data
)
{
    std::string read_buffer = request_to_string(method, url, data);
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
} // namespace curl
} // namespace nutc
