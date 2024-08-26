#pragma once

#include "shared/types/decimal_price.hpp"
#include "shared/types/position.hpp"
#include "shared/types/ticker.hpp"
#include "util.hpp"

#include <fmt/format.h>
#include <glaze/glaze.hpp>

namespace nutc::shared {

/**
 * @brief Contains all types used by glaze and the exchange for orders, matching,
 * communication, etc
 */

struct start_time {
    int64_t start_time_ns;

    start_time() = default;

    explicit start_time(int64_t stns) : start_time_ns(stns) {}
};

/*f
 * @brief Sent by exchange to a client to indicate a match has occurred
 */
struct match {
    shared::position position;
    std::string buyer_id;
    std::string seller_id;
    shared::decimal_price buyer_capital;
    shared::decimal_price seller_capital;

    match() = default;

    match(
        shared::Ticker ticker, shared::Side side, double quantity,
        shared::decimal_price price, std::string bid, std::string sid,
        shared::decimal_price bcap, shared::decimal_price scap
    ) :
        position{ticker, side, quantity, price},
        buyer_id(std::move(bid)), seller_id(std::move(sid)), buyer_capital(bcap),
        seller_capital(scap)
    {}

    match(
        const shared::position& position, std::string bid, std::string sid,
        shared::decimal_price bcap, shared::decimal_price scap
    ) :
        position(position),
        buyer_id(std::move(bid)), seller_id(std::move(sid)), buyer_capital(bcap),
        seller_capital(scap)
    {}
};

struct tick_update {
    std::vector<shared::position> ob_updates;
    std::vector<match> matches;

    tick_update() = default;

    explicit tick_update(
        std::vector<shared::position> ob_updates, std::vector<match> matches
    ) :
        ob_updates(std::move(ob_updates)),
        matches(std::move(matches))
    {}
};

struct algorithm_content {
    std::string algorithm_content_str;

    algorithm_content() = default;

    explicit algorithm_content(std::string algorithm) : algorithm_content_str(algorithm)
    {}
};

} // namespace nutc::shared

/// \cond
template <>
struct glz::meta<nutc::shared::tick_update> {
    using t = nutc::shared::tick_update;
    static constexpr auto value = object("tick_update", &t::ob_updates, &t::matches);
};

/// \cond
template <>
struct glz::meta<nutc::shared::match> {
    using t = nutc::shared::match;
    static constexpr auto value = object(
        "match", &t::position, &t::buyer_id, &t::seller_id, &t::buyer_capital,
        &t::seller_capital
    );
};

/// \cond
template <>
struct glz::meta<nutc::shared::start_time> {
    using t = nutc::shared::start_time;
    static constexpr auto value = // NOLINT
        object("start_time", &t::start_time_ns);
};

/// \cond
template <>
struct glz::meta<nutc::shared::algorithm_content> {
    using t = nutc::shared::algorithm_content;
    static constexpr auto value = // NOLINT
        object("algo", &t::algorithm_content_str);
};
