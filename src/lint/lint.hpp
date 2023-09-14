#pragma once

#include "firebase/fetching.hpp"
#include "firebase/firebase.hpp"
#include "mock_api/mock_api.hpp"
#include "pywrapper/runtime.hpp"

#include <pybind11/pybind11.h>

#include <string>

namespace nutc {
namespace lint {

std::string lint(const std::string& uid, const std::string& algo_id);

} // namespace lint
} // namespace nutc
