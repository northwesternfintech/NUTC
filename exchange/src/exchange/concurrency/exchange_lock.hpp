#pragma once

#include <atomic>

namespace nutc {
namespace concurrency {

/**
 * @brief A simple lock to prevent multiple threads from accessing the exchange at the
 * same time
 */
class ExchangeLock {
    std::atomic_flag flag_{};

    ExchangeLock() = default;
    ~ExchangeLock() = default;

    static ExchangeLock&
    get_instance()
    {
        static ExchangeLock instance;
        return instance;
    }

    void lock_();

    void unlock_();

public:
    /**
     * @brief Block until we can acquire the lock
     */
    static void lock();

    /**
     * @brief Instantly unlock the exchange
     * @note Assumes (and asserts, in dev mode) that it is already locked
     */
    static void unlock();

    ExchangeLock(ExchangeLock const&) = delete;
    ExchangeLock& operator=(ExchangeLock const&) = delete;
    ExchangeLock(ExchangeLock&&) = delete;
    ExchangeLock& operator=(ExchangeLock&&) = delete;
};

} // namespace concurrency
} // namespace nutc
