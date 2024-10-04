#include "cpp_runtime.hpp"

#include <dlfcn.h>
#include <fcntl.h>
#include <fmt/core.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>

#include <filesystem>
#include <fstream>
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

namespace nutc::lint {

CppRuntime::CppRuntime(
    std::string algo, LimitOrderFunction limit_order, MarketOrderFunction market_order,
    CancelOrderFunction cancel_order
) : Runtime(std::move(algo), limit_order, market_order, cancel_order)
{}

CppRuntime::~CppRuntime()
{
    dlclose(dl_handle_);
    close(fd_);
}

std::optional<std::string>
CppRuntime::init()
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

    if (!init_func || !on_trade_update_func_ || !on_orderbook_update_func_
        || !on_account_update_func_) {
        dlclose(dl_handle_);
        close(fd_);
        return fmt::format("[linter] failed to dynamically load functions");
    }
    strategy_object_ =
        init_func(m_market_order_func, m_limit_order_func, m_cancel_order_func);

    return std::nullopt;
}

void
CppRuntime::fire_on_trade_update(
    common::Ticker ticker, common::Side side, double price, double quantity
) const
{
    on_trade_update_func_(
        strategy_object_, ticker, side, static_cast<double>(quantity),
        static_cast<double>(price)
    );
}

void
CppRuntime::fire_on_orderbook_update(
    common::Ticker ticker, common::Side side, double price, double quantity
) const
{
    on_orderbook_update_func_(
        strategy_object_, ticker, side, static_cast<double>(quantity),
        static_cast<double>(price)
    );
}

void
CppRuntime::fire_on_account_update(
    common::Ticker ticker, common::Side side, double price, double quantity,
    double capital
) const
{
    on_account_update_func_(
        strategy_object_, ticker, side, static_cast<double>(quantity),
        static_cast<double>(price), static_cast<double>(capital)
    );
}

} // namespace nutc::lint
