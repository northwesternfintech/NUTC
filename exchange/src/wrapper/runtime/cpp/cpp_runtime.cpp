#include "cpp_runtime.hpp"

#include "common/util.hpp"
#include "wrapper/messaging/exchange_communicator.hpp"

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {

std::pair<int, std::filesystem::path>
get_temp_file()
{
    int memfd = memfd_create("algo", MFD_CLOEXEC);
    if (memfd == -1) {
        throw std::runtime_error("Failed to create memfd");
    }

    return {memfd, "/proc/self/fd/" + std::to_string(memfd)};
}

} //   namespace

namespace nutc::wrapper {

CppRuntime::CppRuntime(
    std::string algo, std::string trader_id, ExchangeCommunicator communicator
) : Runtime(std::move(algo), std::move(trader_id), std::move(communicator))
{
    auto [fd, path] = get_temp_file();

    fd_ = fd;

    std::ofstream algo_file(path);
    algo_file << algo_ << std::flush;
    algo_file.close();

    dl_handle_ = dlopen(path.c_str(), RTLD_NOW);
    if (dl_handle_ == nullptr) {
        std::string err = dlerror();
        close(fd_);
        throw std::runtime_error(fmt::format("Failed to dlopen: {}", err));
    }

    auto init_func = reinterpret_cast<InitFunc>(dlsym(dl_handle_, "init"));
    on_trade_update_func_ =
        reinterpret_cast<on_trade_update_func>(dlsym(dl_handle_, "on_trade_update"));
    on_orderbook_update_func_ = reinterpret_cast<on_orderbook_update_func>(
        dlsym(dl_handle_, "on_orderbook_update")
    );
    on_account_update_func_ =
        reinterpret_cast<on_account_update_func>(dlsym(dl_handle_, "on_account_update")
        );

    if (init_func == nullptr || on_trade_update_func_ == nullptr
        || on_orderbook_update_func_ == nullptr || on_account_update_func_ == nullptr) {
        dlclose(dl_handle_);
        close(fd_);
        throw std::runtime_error("Failed to dynamically load functions");
    }
    strategy_object_ = init_func(
        communicator_.place_market_order(), communicator_.place_limit_order(),
        communicator_.cancel_order(), log_text
    );
}

CppRuntime::~CppRuntime()
{
    dlclose(dl_handle_);
    close(fd_);
}

void
CppRuntime::fire_on_trade_update(
    Ticker ticker, Side side, decimal_quantity quantity, decimal_price price
) const
{
    on_trade_update_func_(
        strategy_object_, ticker, side, static_cast<float>(quantity),
        static_cast<float>(price)
    );
}

void
CppRuntime::fire_on_orderbook_update(
    Ticker ticker, Side side, decimal_quantity quantity, decimal_price price
) const
{
    on_orderbook_update_func_(
        strategy_object_, ticker, side, static_cast<float>(quantity),
        static_cast<float>(price)
    );
}

void
CppRuntime::fire_on_account_update(
    Ticker ticker, Side side, decimal_price price, decimal_quantity quantity,
    decimal_price capital
) const
{
    on_account_update_func_(
        strategy_object_, ticker, side, static_cast<float>(price),
        static_cast<float>(quantity), static_cast<float>(capital)
    );
}

} // namespace nutc::wrapper
