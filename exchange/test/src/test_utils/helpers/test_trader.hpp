#pragma once

#include "exchange/traders/trader_types/trader_interface.hpp"

#include <fmt/format.h>

namespace nutc {
namespace test_utils {

// Basically a generic trader but
class TestTrader : public traders::GenericTrader {
public:
    TestTrader(std::string trader_id, double capital) :
        GenericTrader(trader_id, capital)
    {}
};

} // namespace test_utils
} // namespace nutc
