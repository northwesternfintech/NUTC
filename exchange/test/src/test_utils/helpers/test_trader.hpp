#pragma once

#include "exchange/traders/trader_types/trader_interface.hpp"
#include "shared/file_operations/file_operations.hpp"
#include "signal.h"

#include <fmt/format.h>

#include <filesystem>

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
