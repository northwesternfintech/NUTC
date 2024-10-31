#pragma once

#include "common/types/decimal.hpp"
#include "common/types/position.hpp"

#include <fmt/format.h>
#include <glaze/core/common.hpp>
#include <glaze/core/meta.hpp>

#include <chrono>

namespace nutc::common {

struct start_time {
    std::int64_t start_time_ns;

    start_time() = default;

    explicit start_time(std::chrono::high_resolution_clock::time_point stns) :
        start_time_ns(stns.time_since_epoch().count())
    {}
};

struct account_update {
    common::position trade;
    common::decimal_price available_capital;
};

struct tick_update {
    std::vector<common::position> ob_updates;
    std::vector<common::position> matches;

    tick_update() = default;

    explicit tick_update(
        std::vector<common::position> ob_updates, std::vector<common::position> matches
    ) : ob_updates(std::move(ob_updates)), matches(std::move(matches))
    {}
};

struct algorithm_content {
    std::string algorithm_content_str;

    algorithm_content() = default;

    explicit algorithm_content(std::string algorithm) :
        algorithm_content_str(std::move(algorithm))
    {}
};

} // namespace nutc::common

template <>
struct glz::meta<nutc::common::tick_update> {
    using t = nutc::common::tick_update;
    static constexpr auto value = object("tick_update", &t::ob_updates, &t::matches);
};

template <>
struct glz::meta<nutc::common::account_update> {
    using t = nutc::common::account_update;
    static constexpr auto value =
        object("account_update", &t::trade, &t::available_capital);
};

template <>
struct glz::meta<nutc::common::start_time> {
    using t = nutc::common::start_time;
    static constexpr auto value = // NOLINT
        object("start_time", &t::start_time_ns);
};

template <>
struct glz::meta<nutc::common::algorithm_content> {
    using t = nutc::common::algorithm_content;
    static constexpr auto value = // NOLINT
        object("algo", &t::algorithm_content_str);
};
