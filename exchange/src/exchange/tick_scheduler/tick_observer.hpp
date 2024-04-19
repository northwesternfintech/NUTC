#pragma once

#include <cstdint>

namespace nutc {
namespace ticks {

class TickObserver {
public:
    virtual ~TickObserver() = default;
    TickObserver() = default;
    TickObserver(const TickObserver&) = default;
    TickObserver(TickObserver&&) = default;
    TickObserver& operator=(const TickObserver&) = default;
    TickObserver& operator=(TickObserver&&) = default;

    virtual void on_tick(uint64_t new_tick) = 0;
};
} // namespace ticks
} // namespace nutc
