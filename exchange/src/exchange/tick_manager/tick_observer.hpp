#pragma once

#include <cstdint>

namespace nutc {
namespace ticks {

class TickObserver {
public:
    virtual ~TickObserver() = default;

    virtual void on_tick(uint64_t new_tick) = 0;
};
} // namespace ticks
} // namespace nutc
