#pragma once

#include <tuple>
#include <vector>

namespace nutc {
template <typename T, typename... MessageTypes>
constexpr bool is_one_of = (std::is_same_v<T, MessageTypes> || ...);

template <typename... MessageTypes>
class MessageStorage {
    std::tuple<std::vector<MessageTypes>...> messages;

public:
    template <typename T>
    std::vector<T>
    extract()
    {
        auto& original_vec = get<T>();
        std::vector<T> result = std::move(original_vec);
        original_vec.clear();
        return result;
    }

    template <typename T>
    bool
    empty() const
    {
        return get<T>().empty();
    }

    template <typename T>
    void
    add(const T& message)
    {
        get<T>().push_back(message);
    }

private:
    template <typename T>
    requires is_one_of<T, MessageTypes...>
    std::vector<T>&
    get()
    {
        return std::get<std::vector<T>>(messages);
    }
};
} // namespace nutc
