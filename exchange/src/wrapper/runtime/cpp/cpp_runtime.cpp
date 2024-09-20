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
namespace fs = std::filesystem;

std::pair<int, std::filesystem::path>
get_temp_file()
{
#ifdef __APPLE__
    std::string template_path = (fs::temp_directory_path() / "algoXXXXXX").string();
    std::vector<char> writable_template_path(
        template_path.begin(), template_path.end()
    );
    writable_template_path.push_back('\0');
    int fd = mkstemp(writable_template_path.data());
    if (fd == -1) {
        throw std::runtime_error("Failed to get file descriptor for temporary file");
    }

    return {fd, writable_template_path.data()};

#else
    int memfd = memfd_create("algo", MFD_CLOEXEC);
    if (memfd == -1) {
        throw std::runtime_error("Failed to create memfd");
    }

    return {memfd, "/proc/self/fd/" + std::to_string(memfd)};

#endif
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
        reinterpret_cast<OnTradeUpdateFunc>(dlsym(dl_handle_, "on_trade_update"));
    on_orderbook_update_func_ =
        reinterpret_cast<OnOrderBookUpdateFunc>(dlsym(dl_handle_, "on_orderbook_update")
        );
    on_account_update_func_ =
        reinterpret_cast<OnAccountUpdateFunc>(dlsym(dl_handle_, "on_account_update"));

    if (!init_func || !on_trade_update_func_ || !on_orderbook_update_func_
        || !on_account_update_func_) {
        dlclose(dl_handle_);
        close(fd_);
        throw std::runtime_error("Failed to dynamically load functions");
    }
    strategy_object_ = init_func(
        communicator_.place_market_order(), communicator_.place_limit_order(),
        communicator_.cancel_order()
    );
}

CppRuntime::~CppRuntime()
{
    dlclose(dl_handle_);
    close(fd_);
}

void
CppRuntime::fire_on_trade_update(
    Ticker ticker, Side side, decimal_price price, decimal_quantity quantity
) const
{
    std::string ticker_val{to_string(ticker)};
    std::string side_val = (side == Side::buy) ? "BUY" : "SELL";

    on_trade_update_func_(
        strategy_object_, ticker_val, side_val, static_cast<double>(quantity),
        static_cast<double>(price)
    );
}

void
CppRuntime::fire_on_orderbook_update(
    Ticker ticker, Side side, decimal_price price, decimal_quantity quantity
) const
{
    std::string ticker_val{to_string(ticker)};
    std::string side_val = (side == Side::buy) ? "BUY" : "SELL";

    on_orderbook_update_func_(
        strategy_object_, ticker_val, side_val, static_cast<double>(quantity),
        static_cast<double>(price)
    );
}

void
CppRuntime::fire_on_account_update(
    Ticker ticker, Side side, decimal_price price, decimal_quantity quantity,
    decimal_price capital
) const
{
    std::string ticker_val{to_string(ticker)};
    std::string side_val = (side == Side::buy) ? "BUY" : "SELL";

    on_account_update_func_(
        strategy_object_, ticker_val, side_val, static_cast<double>(quantity),
        static_cast<double>(price), static_cast<double>(capital)
    );
}

} // namespace nutc::wrapper
