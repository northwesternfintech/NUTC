#include "tsq.hpp"

namespace nutc {
namespace tsq {

template <typename T>
void
ThreadSafeQueue<T>::push(const T& value)
{
    std::lock_guard<std::mutex> lock(mtx);
    queue.push(value);
}

template <typename T>
std::optional<T>
ThreadSafeQueue<T>::pop()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (queue.empty()) {
        return std::nullopt;
    }
    T value = queue.front();
    queue.pop();
    return value;
}

template class ThreadSafeQueue<std::pair<std::string, std::string>>;

} // namespace tsq
} // namespace nutc
