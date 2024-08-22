#pragma once

#include "types/decimal_price.hpp"
#include "types/ticker.hpp"
#include "util.hpp"

#include <fmt/format.h>
#include <glaze/glaze.hpp>
#include <glaze/util/type_traits.hpp>

#ifdef __APPLE__
#  include <mach/mach_time.h>
#else
#  include <x86intrin.h>
#endif

namespace nutc {
namespace messages {

struct init_message {
    std::string_view name = "init_message";
};

struct market_order {
    util::Ticker ticker;
    util::Side side;
    double quantity;

    market_order() = default;

    market_order(util::Ticker ticker, util::Side side, double quantity) :
        ticker(ticker), side(side), quantity(quantity)
    {}
};

struct limit_order {
    util::Ticker ticker;
    util::Side side;
    double quantity;
    util::decimal_price price;
    bool ioc;

    bool operator==(const limit_order& other) const = default;

    limit_order(
        util::Ticker ticker, util::Side side, double quantity,
        util::decimal_price price, bool ioc = false
    ) : ticker{ticker}, side{side}, quantity{quantity}, price{price}, ioc{ioc}
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

template <typename OrderT>
struct timed_message : public OrderT {
    uint64_t timestamp;

    template <typename... Args>
    timed_message(Args&&... args) :
        OrderT(std::forward<Args>(args)...), timestamp(get_time())
    {}
};

using timed_init_message = timed_message<init_message>;
using timed_limit_order = timed_message<limit_order>;
using timed_market_order = timed_message<market_order>;

} // namespace messages
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::messages::limit_order> {
    using t = nutc::messages::limit_order;
    static constexpr auto value =
        object("limit", &t::ticker, &t::side, &t::quantity, &t::price, &t::ioc);
};

/// \cond
template <>
struct glz::meta<nutc::messages::market_order> {
    using t = nutc::messages::market_order;
    static constexpr auto value = object("market", &t::ticker, &t::side, &t::quantity);
};

/// \cond
template <>
struct glz::meta<nutc::messages::init_message> {
    using t = nutc::messages::init_message;
    static constexpr auto value = object(&t::name);
};
