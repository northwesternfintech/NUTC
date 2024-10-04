#include "lint.hpp"

#include "linter/runtime/runtime.hpp"

#include <fmt/core.h>
#include <pybind11/pybind11.h>

#include <optional>
#include <stdexcept>
#include <string>

namespace nutc {
namespace lint {

lint_result
lint(Runtime& runtime)
{
    std::string out_message = "[linter] starting to lint algorithm\n";
    auto init_err = runtime.init();
    if (init_err.has_value()) {
        out_message += fmt::format("{}\n", init_err.value());
        return {false, out_message};
    }

    try {
        runtime.fire_on_orderbook_update("ETH", "BUY", 1.0, 1.0);
    } catch (const std::exception& e) {
        out_message += fmt::format("Failed to run on_orderbook_update: {}", e.what());
        return {false, out_message};
    }

    try {
        runtime.fire_on_trade_update("ETH", "BUY", 1.0, 1.0);
    } catch (const std::exception& e) {
        out_message += fmt::format("Failed to run on_trade_update: {}", e.what());
        return {false, out_message};
    }

    try {
        runtime.fire_on_orderbook_update("BTC", "BUY", 1.0, 1.0);
    } catch (const std::exception& e) {
        out_message += fmt::format("Failed to run on_orderbook_update: {}", e.what());
        return {false, out_message};
    }

    try {
        runtime.fire_on_trade_update("BTC", "BUY", 1.0, 1.0);
    } catch (const std::exception& e) {
        out_message += fmt::format("Failed to run on_trade_update: {}", e.what());
        return {false, out_message};
    }

    try {
        runtime.fire_on_account_update("BTC", "BUY", 1.0, 1.0, 1.0);
    } catch (const std::exception& e) {
        out_message += fmt::format("Failed to run on_trade_update: {}", e.what());
        return {false, out_message};
    }

    try {
        runtime.fire_on_account_update("BTC", "BUY", 1.0, 1.0, 1.0);
    } catch (const std::exception& e) {
        out_message += fmt::format("Failed to run on_account_update: {}", e.what());
        return {false, out_message};
    }

    try {
        runtime.fire_on_orderbook_update("LTC", "BUY", 1.0, 1.0);
    } catch (const std::exception& e) {
        out_message += fmt::format("Failed to run on_orderbook_update: {}", e.what());
        return {false, out_message};
    }

    try {
        runtime.fire_on_trade_update("LTC", "BUY", 1.0, 1.0);
    } catch (const std::exception& e) {
        out_message += fmt::format("Failed to run on_trade_update: {}", e.what());
        return {false, out_message};
    }

    try {
        runtime.fire_on_account_update("LTC", "BUY", 1.0, 1.0, 1.0);
    } catch (const std::exception& e) {
        out_message += fmt::format("Failed to run on_account_update: {}", e.what());
        return {false, out_message};
    }

    out_message += "\n[linter] linting process succeeded!\n";
    return {true, out_message};
}

} // namespace lint
} // namespace nutc
