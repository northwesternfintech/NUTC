#pragma once

#include "common/util.hpp"

#include <cstdint>

#include <string>

namespace nutc::wrapper {
struct wrapper_args {
    const uint8_t VERBOSITY;
    const std::string TRADER_ID;
    const common::AlgoType ALGO_TYPE;
};

wrapper_args process_arguments(int argc, const char** argv);
} // namespace nutc::wrapper
