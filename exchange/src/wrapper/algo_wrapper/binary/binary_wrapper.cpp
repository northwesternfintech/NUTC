#include "wrapper/algo_wrapper/binary/binary_wrapper.hpp"

#include "wrapper/algo_wrapper/wrapper.hpp"

#include <dlfcn.h>
#include <fcntl.h>
#include <fmt/core.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <system_error>

namespace nutc {
namespace wrapper {

BinaryWrapper::BinaryWrapper(
    std::string algo, std::string trader_id, MarketOrderFunc publish_market_order
) : Wrapper(std::move(algo), std::move(trader_id))
{
    // Create a memory file descriptor
    memfd_ = memfd_create("wrapper-library", MFD_CLOEXEC);
    if (memfd_ == -1) {
        throw std::system_error(errno, std::generic_category(), "memfd_create failed");
    }

    auto bytes_written = write(memfd_, algo_.data(), algo_.size());
    if (bytes_written == -1 || static_cast<size_t>(bytes_written) != algo_.size()) {
        close(memfd_);
        throw std::system_error(
            errno, std::generic_category(), "write to memfd failed"
        );
    }

    std::string lib_path = "/proc/self/fd/" + std::to_string(memfd_);
    dl_handle_ = dlopen(lib_path.c_str(), RTLD_LAZY);
    if (!dl_handle_) {
        close(memfd_);
        throw std::runtime_error("dlopen failed: " + std::string(dlerror()));
    }

    auto create_strategy = reinterpret_cast<StrategyObject* (*)(MarketOrderFunc)>(
        dlsym(dl_handle_, "create_strategy")
    );
    if (!create_strategy) {
        dlclose(dl_handle_);
        close(memfd_);
        throw std::runtime_error(
            "dlsym failed for create_strategy: " + std::string(dlerror())
        );
    }

    delete_strategy_ =
        reinterpret_cast<DeleteStrategyFunc>(dlsym(dl_handle_, "delete_strategy"));
    if (!delete_strategy_) {
        dlclose(dl_handle_);
        close(memfd_);
        throw std::runtime_error(
            "dlsym failed for delete_strategy: " + std::string(dlerror())
        );
    }

    on_trade_update_ =
        reinterpret_cast<OnTradeUpdateFunc>(dlsym(dl_handle_, "on_trade_update"));
    if (!on_trade_update_) {
        dlclose(dl_handle_);
        close(memfd_);
        throw std::runtime_error(
            "dlsym failed for on_trade_update: " + std::string(dlerror())
        );
    }

    on_orderbook_update_ =
        reinterpret_cast<OnOrderBookUpdateFunc>(dlsym(dl_handle_, "on_orderbook_update")
        );
    if (!on_orderbook_update_) {
        dlclose(dl_handle_);
        close(memfd_);
        throw std::runtime_error(
            "dlsym failed for on_orderbook_update: " + std::string(dlerror())
        );
    }

    on_account_update_ =
        reinterpret_cast<OnAccountUpdateFunc>(dlsym(dl_handle_, "on_account_update"));
    if (!on_account_update_) {
        dlclose(dl_handle_);
        close(memfd_);
        throw std::runtime_error(
            "dlsym failed for on_account_update: " + std::string(dlerror())
        );
    }

    strategy_object_ = create_strategy(publish_market_order);
}

BinaryWrapper::~BinaryWrapper()
{
    delete_strategy_(strategy_object_);

    dlclose(dl_handle_);
    close(memfd_);
}

void
BinaryWrapper::fire_on_trade_update(
    std::string ticker, std::string side, double price, double quantity
) const
{
    on_trade_update_(strategy_object_, ticker, side, price, quantity);
}

void
BinaryWrapper::fire_on_orderbook_update(
    std::string ticker, std::string side, double price, double quantity
) const
{
    on_orderbook_update_(strategy_object_, ticker, side, price, quantity);
}

void
BinaryWrapper::fire_on_account_update(
    std::string ticker, std::string side, double price, double quantity,
    double buyer_capital
) const
{
    on_account_update_(strategy_object_, ticker, side, price, quantity, buyer_capital);
}

} // namespace wrapper
} // namespace nutc
