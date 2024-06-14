#pragma once

#include <cstdint>

#include <functional>

namespace nutc {
namespace ticks {

void run(const std::function<void(uint64_t)> function, uint16_t tick_hz);

} // namespace ticks
} // namespace nutc
