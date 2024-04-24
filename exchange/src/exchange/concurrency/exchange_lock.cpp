#include "exchange_lock.hpp"

#include <cassert>

namespace nutc {
namespace concurrency {

void
ExchangeLock::lock_()
{
    while (flag_.test_and_set(std::memory_order_acquire)) {}
}

void
ExchangeLock::unlock_()
{
    flag_.clear(std::memory_order_release);
}

void
ExchangeLock::lock()
{
    get_instance().lock_();
}

void
ExchangeLock::unlock()
{
    get_instance().unlock_();
}

} // namespace concurrency
} // namespace nutc
