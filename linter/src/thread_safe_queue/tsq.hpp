#pragma once

#include <mutex>
#include <optional>
#include <queue>
#include <string>

namespace nutc {
namespace tsq {

template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    std::mutex mtx;

public:
    void push(const T& value);

    std::optional<T> pop();
};
} // namespace tsq
} // namespace nutc
