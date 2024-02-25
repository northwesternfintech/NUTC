#pragma once

#include <cstddef>

#include <string>

namespace nutc {
namespace concurrency {

void pin_to_core(size_t core_num, std::string thread_name = "");

} // namespace concurrency
} // namespace nutc
