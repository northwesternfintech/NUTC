#pragma once

#include <glaze/glaze.hpp>

#include <cmath>

#include <string>

namespace nutc {
namespace util {

std::string trader_id(const std::string& user_id, const std::string& algo_id);

std::string get_firebase_endpoint(const std::string& params);

enum class Side { buy, sell };
std::string to_string(Side side);

enum class Mode { dev, normal, bots_only };

struct algorithm_content {
    const std::string algorithm;
};

struct algorithm {
    const std::string UID;
    const std::string ALGO_ID;
};

constexpr bool
is_close_to_zero(double value, double epsilon = 1e-6)
{
    return std::fabs(value) < epsilon;
}

} // namespace util
} // namespace nutc

using Algorithm = nutc::util::algorithm_content;

namespace glz {

template <>
struct meta<Algorithm> {
    static constexpr auto value = object("algorithm", &Algorithm::algorithm);
};

} // namespace glz
