#include "cpp_runtime.hpp"

#include "common/compilation/compile_cpp.hpp"

#include <dlfcn.h>
#include <fcntl.h>
#include <fmt/core.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>

#include <fstream>
#include <iostream>

namespace nutc::lint {

CppRuntime::CppRuntime(
    std::string algo, LimitOrderFunction limit_order, MarketOrderFunction market_order,
    CancelOrderFunction cancel_order
) :
    Runtime(
        std::move(algo), std::move(limit_order), std::move(market_order),
        std::move(cancel_order)
    )
{}

CppRuntime::~CppRuntime()
{
    if (dl_handle_ != nullptr) {
        dlclose(dl_handle_);
    }
}

std::optional<std::string>
CppRuntime::init()
{
    boost::filesystem::path temp_dir = boost::filesystem::temp_directory_path();
    boost::filesystem::path temp_file =
        temp_dir / boost::filesystem::unique_path("tempfile-%%%%-%%%%");

    std::ofstream algo_file(temp_file.string());
    algo_file << algo_ << std::flush;
    algo_file.close();

    // TODO: improve
    std::string compiled_binary_path;
    try {
        compiled_binary_path = common::compile_cpp(temp_file.string());
    } catch (const std::exception& e) {
        return fmt::format("[linter] failed to compile C++ code: {}", e.what());
    }

    dl_handle_ = dlopen(compiled_binary_path.c_str(), RTLD_NOW);
    if (dl_handle_ == nullptr) {
        std::string err = dlerror();
        return fmt::format("[linter] failed to dlopen: {}", err);
    }

    auto init_func = reinterpret_cast<InitFunc>(dlsym(dl_handle_, "init"));
    on_trade_update_func_ =
        reinterpret_cast<OnTradeUpdateFunc>(dlsym(dl_handle_, "on_trade_update"));
    on_orderbook_update_func_ =
        reinterpret_cast<OnOrderBookUpdateFunc>(dlsym(dl_handle_, "on_orderbook_update")
        );
    on_account_update_func_ =
        reinterpret_cast<OnAccountUpdateFunc>(dlsym(dl_handle_, "on_account_update"));

    if (init_func == nullptr || on_trade_update_func_ == nullptr
        || on_orderbook_update_func_ == nullptr || on_account_update_func_ == nullptr) {
        return fmt::format("[linter] failed to dynamically load functions");
    }
    strategy_object_ =
        init_func(m_market_order_func, m_limit_order_func, m_cancel_order_func);

    return std::nullopt;
}

void
CppRuntime::fire_on_trade_update(
    common::Ticker ticker, common::Side side, float price, float quantity
) const
{
    on_trade_update_func_(strategy_object_, ticker, side, quantity, price);
}

void
CppRuntime::fire_on_orderbook_update(
    common::Ticker ticker, common::Side side, float price, float quantity
) const
{
    on_orderbook_update_func_(strategy_object_, ticker, side, quantity, price);
}

void
CppRuntime::fire_on_account_update(
    common::Ticker ticker, common::Side side, float price, float quantity, float capital
) const
{
    on_account_update_func_(strategy_object_, ticker, side, quantity, price, capital);
}

} // namespace nutc::lint
