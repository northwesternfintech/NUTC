#pragma once

namespace nutc {
namespace ticks {

class TickObserver {
public:
    virtual ~TickObserver() = default;

    virtual void on_tick() = 0;
};
} // namespace ticks
} // namespace nutc
