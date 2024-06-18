#include "fetching.hpp"

#include "config.h"

#include <curl/curl.h>
#include <fmt/core.h>

#include <optional>
#include <regex>

namespace nutc {
namespace client {

static std::string
get_firebase_endpoint(const std::string& params)
{
#ifdef NUTC_LOCAL_DEV
    return FIREBASE_URL + params + "?ns=nutc-web-default-rtdb";
#else
    return FIREBASE_URL + params;
#endif
}

void
set_lint_result(const std::string& uid, const std::string& algo_id, bool succeeded)
{
    std::string success = "\"success\"";
    std::string failure = "\"failure\"";
    std::string params =
        fmt::format("users/{}/algos/{}/lintResults.json", uid, algo_id);

    firebase_request(
        "PUT", get_firebase_endpoint(params), succeeded ? success : failure
    );
}

std::string
replaceDisallowedValues(const std::string& input)
{
    std::regex newlinePattern("\\n");
    std::string input2 = std::regex_replace(input, newlinePattern, "\\n");
    std::regex disallowedPattern("[.$#\\[\\]]");

    return std::regex_replace(input2, disallowedPattern, "");
}

void
set_lint_success(
    const std::string& uid, const std::string& algo_id, const std::string& success
)
{
    std::string json_success = "\"" + replaceDisallowedValues(success) + "\"";
    std::string params1 =
        fmt::format("users/{}/algos/{}/lintSuccessMessage.json", uid, algo_id);
    std::string params2 = fmt::format("users/{}/latestAlgoId.json", uid);

    firebase_request("PUT", get_firebase_endpoint(params1), json_success);

    firebase_request("PUT", get_firebase_endpoint(params2), "\"" + algo_id + "\"");
    set_lint_result(uid, algo_id, true);
}

void
set_lint_failure(
    const std::string& uid, const std::string& algo_id, const std::string& failure
)
{
    std::string json_failure = "\"" + replaceDisallowedValues(failure) + "\"";
    std::string params =
        fmt::format("users/{}/algos/{}/lintFailureMessage.json", uid, algo_id);
    firebase_request("PUT", get_firebase_endpoint(params), json_failure);
    set_lint_result(uid, algo_id, false);
}

std::optional<glz::json_t>
get_user_info(const std::string& uid)
{
    std::string url = fmt::format("users/{}.json", uid);
    return firebase_request("GET", get_firebase_endpoint(url));
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
storage_request(const std::string& firebase_url)
{
    std::string readBuffer;

    CURL* curl = curl_easy_init();
    if (curl) {
        CURLcode res = curl_easy_setopt(curl, CURLOPT_URL, firebase_url.c_str());
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
get_algo(const std::string& uid, const std::string& algo_id)
{
    auto maybe_user_info = get_user_info(uid);
    if (!maybe_user_info.has_value()) {
        return std::nullopt;
    }
    auto user_info = maybe_user_info.value();
    // if not has "algos"
    if (!user_info.contains("algos") || !user_info["algos"].contains(algo_id)) {
        return std::nullopt;
    }
    glz::json_t algo_info = user_info["algos"][algo_id];
    std::string downloadURL = algo_info["downloadURL"].get<std::string>();
    auto algo_file = storage_request(downloadURL);
    return algo_file;
}

nutc::client::LintingResultOption
get_algo_status(const std::string& uid, const std::string& algo_id)
{
    auto maybe_user_info = get_user_info(uid);
    if (!maybe_user_info.has_value()) {
        return nutc::client::LintingResultOption::UNKNOWN;
    }
    auto user_info = maybe_user_info.value();
    // if not has "algos"
    if (!user_info.contains("algos")) {
        return nutc::client::LintingResultOption::UNKNOWN;
    }

    // check if algo id exists
    if (!user_info["algos"].contains(algo_id)) {
        return nutc::client::LintingResultOption::UNKNOWN;
    }
    glz::json_t algo_info = user_info["algos"][algo_id];

    // check if this algo id has lint results
    if (!algo_info.contains("lintResults")) {
        return nutc::client::LintingResultOption::UNKNOWN;
    }

    std::string linting_result = algo_info["lintResults"].get<std::string>();

    switch (linting_result[0]) {
        case 'f':
            return nutc::client::LintingResultOption::FAILURE;
        case 's':
            return nutc::client::LintingResultOption::SUCCESS;
        default:
            return nutc::client::LintingResultOption::PENDING;
    }
}

std::optional<glz::json_t>
firebase_request(
    const std::string& method, const std::string& url, const std::string& data
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

        if (method == "POST") {
            res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            if (res != CURLE_OK) {
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
