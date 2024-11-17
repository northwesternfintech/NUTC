#pragma once

#include <cstdint>

#include <string>
#include <string_view>

namespace nutc {

constexpr std::string DEFAULT_CONFIG_FILE = "config.yml";
constexpr uint8_t DEBUG_NUM_USERS = 2;

// How many outgoing messages for one wrapper before we start dropping
constexpr uint32_t MAX_OUTGOING_MQ_SIZE = 10000;

// Limit to 16kb
constexpr uint32_t MAX_PIPE_MSG_SIZE = 16000;

constexpr std::string ALGO_DIR = "algos";

constexpr float RETAIL_ORDER_OFFSET = .02f;
// How much retail orders are offset from theo price. Ex. .02 means buy order is
// theo+.02 to ensure it gets filled

constexpr float RETAIL_ORDER_SIZE = .05f;
// How much of the interest limit to use for retail orders
} // namespace nutc
