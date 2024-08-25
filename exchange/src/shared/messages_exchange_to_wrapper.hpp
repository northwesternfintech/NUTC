#pragma once

#include "shared/types/decimal_price.hpp"
#include "shared/types/position.hpp"
#include "shared/types/ticker.hpp"
#include "util.hpp"

#include <fmt/format.h>
#include <glaze/glaze.hpp>

namespace nutc {

/**
 * @brief Contains all types used by glaze and the exchange for orders, matching,
 * communication, etc
 */
namespace messages {

struct start_time {
    int64_t start_time_ns;

    start_time() = default;

    explicit start_time(int64_t stns) : start_time_ns(stns) {}
};

/*f
 * @brief Sent by exchange to a client to indicate a match has occurred
 */
struct match {
    util::position position;
    std::string buyer_id;
    std::string seller_id;
    util::decimal_price buyer_capital;
    util::decimal_price seller_capital;

    match() = default;

    match(
        util::Ticker ticker, util::Side side, double quantity,
        util::decimal_price price, std::string bid, std::string sid,
        util::decimal_price bcap, util::decimal_price scap
    ) :
        position{ticker, side, quantity, price},
        buyer_id(std::move(bid)), seller_id(std::move(sid)), buyer_capital(bcap),
        seller_capital(scap)
    {}

    match(
        const util::position& position, std::string bid, std::string sid,
        util::decimal_price bcap, util::decimal_price scap
    ) :
        position(position),
        buyer_id(std::move(bid)), seller_id(std::move(sid)), buyer_capital(bcap),
        seller_capital(scap)
    {}
};

struct tick_update {
    std::vector<util::position> ob_updates;
    std::vector<match> matches;

    tick_update() = default;

    explicit tick_update(
        std::vector<util::position> ob_updates, std::vector<match> matches
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

} // namespace messages
} // namespace nutc

/// \cond
template <>
struct glz::meta<nutc::messages::tick_update> {
    using t = nutc::messages::tick_update;
    static constexpr auto value = object( // NOLINT
        &t::ob_updates, &t::matches
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::match> {
    using t = nutc::messages::match;
    static constexpr auto value = object( // NOLINT
        &t::position, &t::buyer_id, &t::seller_id, &t::buyer_capital, &t::seller_capital
    );
};

/// \cond
template <>
struct glz::meta<nutc::messages::start_time> {
    using t = nutc::messages::start_time;
    static constexpr auto value = // NOLINT
        object(&t::start_time_ns);
};

/// \cond
template <>
struct glz::meta<nutc::messages::algorithm_content> {
    using t = nutc::messages::algorithm_content;
    static constexpr auto value = // NOLINT
        object(&t::algorithm_content_str);
};
