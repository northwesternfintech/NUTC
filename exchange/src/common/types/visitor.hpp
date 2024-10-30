#pragma once

namespace nutc::common {

template <typename... Callables>
struct visitor : Callables... {
    using Callables::operator()...;

    explicit visitor(Callables... callables) : Callables(callables)... {}
};

template <typename... Callables>
visitor<Callables...>
make_visitor(Callables... callables)
{
    return visitor<Callables...>(callables...);
}
} // namespace nutc::common
