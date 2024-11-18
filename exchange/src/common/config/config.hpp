#pragma once

// Compile-time config shared between exchange and wrapper

#include <cstdint>

#include <string>

namespace nutc::common {
constexpr std::string NUTC_VERSION = "1.0";

// logging
constexpr uint8_t LOG_BACKTRACE_SIZE = 10;

// Constexpr std::string concat needs llvm 19, which is annoying to install
constexpr std::string LOG_DIR = "logs";
constexpr std::string LOG_FILEPATH = "logs/app.log";
constexpr std::string JSON_LOG_FILEPATH = "logs/json.log";

constexpr uint32_t LOG_FILE_SIZE = 1024 * 1024 / 2; // 512 KB
constexpr uint8_t LOG_BACKUP_COUNT = 5;
} // namespace nutc::common
