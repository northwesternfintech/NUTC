#pragma once

#include <cstddef>

namespace nutc::wrapper {
void kill_on_exchange_death();

void set_memory_limit(std::size_t limit_in_mb);
} // namespace nutc::wrapper
