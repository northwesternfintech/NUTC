#pragma once
#include "messages_exchange_to_wrapper.hpp"

#include <glaze/glaze.hpp>

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
