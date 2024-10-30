#pragma once

#include "common/types/decimal.hpp"
#include "common/types/position.hpp"

#include <fmt/format.h>

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
