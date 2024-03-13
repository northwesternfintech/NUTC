#pragma once

#include <atomic>

namespace nutc {
namespace concurrency {

class ExchangeLock {
    std::atomic_flag flag_;

    ExchangeLock() : flag_(ATOMIC_FLAG_INIT) {}

    ~ExchangeLock() = default;

public:
    ExchangeLock(ExchangeLock const&) = delete;
    ExchangeLock& operator=(ExchangeLock const&) = delete;
    ExchangeLock(ExchangeLock&&) = delete;
    ExchangeLock& operator=(ExchangeLock&&) = delete;

    static ExchangeLock&
    get_instance()
    {
        static ExchangeLock instance;
        return instance;
    }

    void
    lock()
    {
        while (flag_.test_and_set(std::memory_order_acquire)) {}
    }

    void
    unlock()
    {
        flag_.clear(std::memory_order_release);
    }
};

} // namespace concurrency
} // namespace nutc
