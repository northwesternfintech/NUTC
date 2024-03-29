#pragma once

#include "firebase/fetching.hpp"
#include "mock_api/mock_api.hpp"
#include "pywrapper/runtime.hpp"

#include <pybind11/pybind11.h>

#include <string>

namespace nutc {
namespace lint {

[[nodiscard]] bool lint(
    const std::string& uid, const std::string& algo_id, std::stringstream& output_stream
);

} // namespace lint
} // namespace nutc
