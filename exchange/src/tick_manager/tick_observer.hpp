#pragma once

namespace nutc {
namespace ticks {

class TickObserver {
public:
    virtual ~TickObserver() {}

    virtual void onTick() = 0;
};
} // namespace ticks
} // namespace nutc
