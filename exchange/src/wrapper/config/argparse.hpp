#pragma once

#include <cstdint>

#include <string>

namespace nutc::config {
struct wrapper_args {
    const uint8_t VERBOSITY;
    const std::string TRADER_ID;
};

wrapper_args process_arguments(int argc, const char** argv);
} // namespace nutc::config
