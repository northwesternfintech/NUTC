#pragma once

#include "exchange/orders/storage/decimal_price.hpp"
#include "hash_table7.hpp"
#include "shared/util.hpp"

#include <vector>

namespace nutc {

namespace matching {
class LevelQuantityTracker {
    std::vector<double> bid_levels_{100};
    std::vector<double> ask_levels_{100};
    emhash7::HashMap<uint32_t, double> overflow_bid_levels_;
    emhash7::HashMap<uint32_t, double> overflow_ask_levels_;

public:
    void
    report_quantity(util::Side side, decimal_price price, double delta)
    {
        // $500
        if (price.price < 500'00) [[likely]] {
            report_small_quantity(side, price, delta);
        }
        else {
            report_large_quantity(side, price, delta);
        }
    }

    double
    get_level(util::Side side, decimal_price price) const
    {
        // $500
        if (price.price < 500'00) [[likely]] {
            return get_small_level(side, price);
        }
        else {
            return get_large_level(side, price);
        }
    }

private:
    void report_large_quantity(util::Side side, decimal_price price, double delta);
    void report_small_quantity(util::Side side, decimal_price price, double delta);

    double get_large_level(util::Side side, decimal_price price) const;
    double get_small_level(util::Side side, decimal_price price) const;
};

} // namespace matching
} // namespace nutc
