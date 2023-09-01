#pragma once

#include "mock_api/mock_api.hpp"
#include <string>
#include "pywrapper/pywrapper.hpp"
#include <pybind11/pybind11.h>
#include "firebase/firebase.hpp"

namespace nutc {
namespace lint {

std::string lint(const std::string& uid, const std::string& algo_id);

} // namespace server
} // namespace nutc
