#pragma once

#include "common/types/decimal.hpp"
#include "common/types/position.hpp"

#include <fmt/format.h>
#include <glaze/core/common.hpp>
#include <glaze/core/meta.hpp>

namespace nutc::common {

struct start_time {
    int64_t start_time_ns;

    start_time() = default;

    explicit start_time(int64_t stns) : start_time_ns(stns) {}
};

struct match {
    common::position position;
    std::string buyer_id;
    std::string seller_id;
    common::decimal_price buyer_capital;
    common::decimal_price seller_capital;
    std::string match_type{};

    match() = default;

    match(
        const common::position& position, std::string bid, std::string sid,
        common::decimal_price bcap, common::decimal_price scap
    ) :
        position(position), buyer_id(std::move(bid)), seller_id(std::move(sid)),
        buyer_capital(bcap), seller_capital(scap)
    {}
};

struct tick_update {
    std::vector<common::position> ob_updates;
    std::vector<match> matches;

    tick_update() = default;

    explicit tick_update(
        std::vector<common::position> ob_updates, std::vector<match> matches
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
struct glz::meta<nutc::common::match> {
    using t = nutc::common::match;
    static constexpr auto value = object(
        "match", &t::position, &t::buyer_id, &t::seller_id, &t::buyer_capital,
        &t::seller_capital
    );
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
