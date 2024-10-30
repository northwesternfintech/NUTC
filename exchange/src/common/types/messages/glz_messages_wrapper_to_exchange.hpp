#pragma once
#include "messages_wrapper_to_exchange.hpp"

#include <glaze/glaze.hpp>
#include <glaze/util/type_traits.hpp>

/// \cond
template <>
struct glz::meta<nutc::common::cancel_order> {
    using t = nutc::common::cancel_order;
    static constexpr auto value =
        object("cancel", &t::ticker, &t::order_id, &t::timestamp);
};

/// \cond
template <>
struct glz::meta<nutc::common::limit_order> {
    using t = nutc::common::limit_order;
    static constexpr auto value = object(
        "limit", &t::ticker, &t::side, &t::quantity, &t::timestamp, &t::price, &t::ioc,
        &t::order_id
    );
};

/// \cond
template <>
struct glz::meta<nutc::common::market_order> {
    using t = nutc::common::market_order;
    static constexpr auto value =
        object("market", &t::timestamp, &t::ticker, &t::side, &t::quantity);
};

/// \cond
template <>
struct glz::meta<nutc::common::init_message> {
    using t = nutc::common::init_message;
    static constexpr auto value = object(&t::name);
};
