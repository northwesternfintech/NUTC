#pragma once

#include <functional>
#include <optional>
#include <string>

namespace nutc {
namespace pywrapper {
[[nodiscard]] bool create_api_module(
    std::function<bool(const std::string&, const std::string&, float, float)>
        publish_market_order
);
[[nodiscard]] bool supress_stdout();
[[nodiscard]] std::optional<std::string> import_py_code(const std::string& code);

[[nodiscard]] std::optional<std::string> run_initialization();

[[nodiscard]] std::optional<std::string> trigger_callbacks();
} // namespace pywrapper
} // namespace nutc
