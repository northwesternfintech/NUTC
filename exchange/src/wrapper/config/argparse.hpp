#pragma once

#include <cstdint>

#include <string>

namespace nutc::config {
struct wrapper_args {
    const uint8_t VERBOSITY;
    const std::string USER_ID;
    const std::string ALGO_ID;
    const bool DEV_MODE;
};

wrapper_args process_arguments(int argc, const char** argv);
} // namespace nutc::config
