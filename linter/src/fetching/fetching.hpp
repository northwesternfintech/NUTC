#pragma once

#include <curl/curl.h>
#include <glaze/glaze.hpp>

#include <optional>
#include <string>

namespace nutc {
namespace client {

struct SetLintBody {
    bool success;
    std::string message;
};

std::optional<std::string> storage_request(const std::string& url);
std::string replaceDisallowedValues(const std::string& input);

} // namespace client
} // namespace nutc

template <>
struct glz::meta<nutc::client::SetLintBody> {
    using t = nutc::client::SetLintBody;
    static constexpr auto value =
        object("success", &t::success, "message", &t::message);
};
