#pragma once

#include <glaze/glaze.hpp>

#include <cassert>

#include <string>

namespace nutc::common {
using order_id_t = std::int64_t;
order_id_t generate_order_id();

std::uint64_t get_time();
std::string trader_id(const std::string& user_id, const std::string& algo_id);

std::string get_firebase_endpoint(const std::string& params);

// NOTE: this must be the same as Side in template.hpp
enum class Side { buy = 0, sell = 1 };

std::string to_string(Side side);

std::string base64_encode(const std::string& data);

std::string base64_decode(const std::string& data);

enum class Mode { dev, sandbox, normal, bots_only };

} // namespace nutc::common
