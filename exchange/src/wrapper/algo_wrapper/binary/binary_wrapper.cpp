#include "wrapper/algo_wrapper/binary/binary_wrapper.hpp"

#include "wrapper/algo_wrapper/wrapper.hpp"

#include <dlfcn.h>
#include <fcntl.h>
#include <fmt/core.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

namespace nutc {
namespace wrapper {

BinaryWrapper::BinaryWrapper(
    std::string algo, std::string trader_id, MarketOrderFunc publish_market_order
) : Wrapper(std::move(algo), std::move(trader_id))
{
    int memfd = memfd_create("wrapper-library", MFD_CLOEXEC);
    // TODO: handle error

    auto bytes_written = write(memfd, algo_.data(), algo_.size());
    // TODO: handle error

    std::string lib_path = fmt::format("/proc/self/fd/{}", memfd);
    dl_handle_ = dlopen(lib_path.c_str(), RTLD_LAZY);
    // TODO: handle error

    auto create_strategy = reinterpret_cast<StrategyObject* (*)(MarketOrderFunc)>(
        dlsym(dl_handle_, "create_strategy")
    );
    strategy_object_ = create_strategy(publish_market_order);

    delete_strategy_ =
        reinterpret_cast<DeleteStrategyFunc>(dlsym(dl_handle_, "delete_strategy"));
    on_trade_update_ = reinterpret_cast<
        void (*)(StrategyObject*, std::string, std::string, double, double)>(
        dlsym(dl_handle_, "on_trade_update")
    );
    on_orderbook_update_ = reinterpret_cast<
        void (*)(StrategyObject*, std::string, std::string, double, double)>(
        dlsym(dl_handle_, "on_orderbook_update")
    );
    on_account_update_ = reinterpret_cast<
        void (*)(StrategyObject*, std::string, std::string, double, double, double)>(
        dlsym(dl_handle_, "on_account_update")
    );
}

BinaryWrapper::~BinaryWrapper()
{
    delete_strategy_(strategy_object_);

    dlclose(dl_handle_);
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
