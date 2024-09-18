#pragma once

#include "common/types/algorithm.hpp"

#include <cstdint>

#include <string>

namespace nutc::wrapper {
struct wrapper_args {
    const uint8_t VERBOSITY;
    const std::string TRADER_ID;
    const common::AlgoLanguage ALGO_TYPE;
};

wrapper_args process_arguments(int argc, const char** argv);
} // namespace nutc::wrapper
