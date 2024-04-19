#pragma once

#include <cstddef>

#include <string>

namespace nutc {
namespace concurrency {

void pin_to_core(size_t core_num, const std::string& thread_name);
void pin_to_core(size_t core_num);

} // namespace concurrency
} // namespace nutc
