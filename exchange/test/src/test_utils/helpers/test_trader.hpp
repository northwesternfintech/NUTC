#pragma once

#include "exchange/traders/trader_types/algo_trader.hpp"
#include "shared/file_operations/file_operations.hpp"
#include "signal.h"

#include <fmt/format.h>

#include <filesystem>

namespace nutc {
namespace test_utils {

namespace bp = boost::process;

class TestTrader : public traders::LocalTrader {
    bool has_algo{};

public:
    TestTrader(std::string trader_id, double capital) :
        LocalTrader(trader_id, trader_id, trader_id, capital)
    {}

    TestTrader(std::filesystem::path algo_path, double capital) :
        LocalTrader(std::move(algo_path), capital), has_algo(true)
    {
        if (!std::filesystem::exists(algo_path)) [[unlikely]] {
            std::string err_str = fmt::format(
                "Unable to find local algorithm file: {}", std::string{algo_path}
            );
            throw std::runtime_error(err_str);
        }
    }

    traders::TraderType
    get_type() const override
    {
        return has_algo ? traders::TraderType::local : traders::TraderType::bot;
    }

    bool
    can_leverage() const override
    {
        return false;
    }

    const std::string&
    get_algo_id() const override
    {
        if (!has_algo)
            throw std::runtime_error(
                "Test trader constructed without algo, but get_algo_id called"
            );
        return get_id();
    }
};

} // namespace test_utils
} // namespace nutc
