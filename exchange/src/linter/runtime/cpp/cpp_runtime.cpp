#include "cpp_runtime.hpp"

#include "common/compilation/compile_cpp.hpp"
#include "common/file_operations/file_operations.hpp"

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
    // TODO: shoudl not do
    dlclose(dl_handle_);
    close(fd_);
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

    std::string compiled_binary_path = common::compile_cpp(temp_file.string());

    dl_handle_ = dlopen(compiled_binary_path.c_str(), RTLD_NOW);
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

    if (init_func == nullptr || on_trade_update_func_ == nullptr
        || on_orderbook_update_func_ == nullptr || on_account_update_func_ == nullptr) {
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
