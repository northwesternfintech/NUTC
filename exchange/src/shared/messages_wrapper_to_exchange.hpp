#pragma once

#include "types/decimal_price.hpp"
#include "types/ticker.hpp"
#include "util.hpp"

#include <fmt/format.h>
#include <glaze/glaze.hpp>
#include <glaze/util/type_traits.hpp>

#include <stdexcept>

#ifdef __APPLE__
#  include <mach/mach_time.h>
#else
#  include <x86intrin.h>
#endif

namespace nutc::shared {

struct init_message {
    std::string_view name = "init_message";
};

struct market_order {
    shared::Ticker ticker;
    shared::Side side;
    double quantity;

    market_order() = default;

    market_order(shared::Ticker ticker, shared::Side side, double quantity) :
        ticker(ticker), side(side), quantity(quantity)
    {}
};

struct limit_order {
    shared::Ticker ticker;
    shared::Side side;
    double quantity;
    shared::decimal_price price;
    bool ioc;

    bool operator==(const limit_order& other) const = default;

    consteval limit_order(
        std::string_view ticker, shared::Side side, double quantity,
        shared::decimal_price price, bool ioc = false
    ) :
        ticker{shared::force_to_ticker(ticker)},
        side{side}, quantity{quantity}, price{price}, ioc{ioc}
    {}

    limit_order(
        shared::Ticker ticker, shared::Side side, double quantity,
        shared::decimal_price price, bool ioc = false
    ) :
        ticker{ticker},
        side{side}, quantity{quantity}, price{price}, ioc{ioc}
    {}

    limit_order() = default;
};

namespace {
inline uint64_t
get_time()
{
#ifdef __APPLE__
    static uint64_t min_time = 0;
    return min_time = std::max(min_time + 1, mach_absolute_time());
#else
    return __rdtsc();
#endif
}
} // namespace

template <typename ordered>
struct timestamped_message : public ordered {
    uint64_t timestamp;

    timestamped_message() = delete;

    template <typename... Args>
    explicit timestamped_message(Args&&... args) :
        ordered(std::forward<Args>(args)...), timestamp(get_time())
    {}
};

using timed_init_message = timestamped_message<init_message>;
using timed_limit_order = timestamped_message<limit_order>;
using timed_market_order = timestamped_message<market_order>;

} // namespace nutc::shared

/// \cond
template <>
struct glz::meta<nutc::shared::limit_order> {
    using t = nutc::shared::limit_order;
    static constexpr auto value =
        object("limit", &t::ticker, &t::side, &t::quantity, &t::price, &t::ioc);
};

/// \cond
template <>
struct glz::meta<nutc::shared::market_order> {
    using t = nutc::shared::market_order;
    static constexpr auto value = object("market", &t::ticker, &t::side, &t::quantity);
};

/// \cond
template <>
struct glz::meta<nutc::shared::init_message> {
    using t = nutc::shared::init_message;
    static constexpr auto value = object("init", &t::name);
};
