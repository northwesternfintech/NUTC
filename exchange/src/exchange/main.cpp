#include "algos/algo_manager.hpp"
#include "common/util.hpp"
#include "exchange/algos/algo_manager.hpp"
#include "exchange/config/dynamic/argparse.hpp"
#include "exchange/config/dynamic/config.hpp"
#include "common/logging/logging.hpp"
#include "exchange/matching_cycle/base/base_cycle.hpp"
#include "exchange/matching_cycle/cycle_interface.hpp"
#include "exchange/matching_cycle/dev/dev_cycle.hpp"
#include "exchange/matching_cycle/sandbox/sandbox_cycle.hpp"
#include "exchange/traders/trader_container.hpp"

#include <csignal>

#include <utility>

namespace {
using namespace nutc::exchange; // NOLINT

std::unique_ptr<MatchingCycleInterface>
create_cycle(TraderContainer& traders, const auto& mode)
{
    using nutc::common::Mode;
    const auto& ticker_config = Config::get().get_tickers();
    double order_fee = Config::get().constants().ORDER_FEE;
    auto tickers = TickerContainer(ticker_config, traders);

    switch (mode) {
        case Mode::normal:
            return std::make_unique<BaseMatchingCycle>(tickers, traders, order_fee);
        case Mode::sandbox:
            return std::make_unique<SandboxMatchingCycle>(tickers, traders, order_fee);
        case Mode::bots_only:
        case Mode::dev:
            return std::make_unique<DevMatchingCycle>(tickers, traders, order_fee);
    }

    std::unreachable();
}

void
main_event_loop(std::unique_ptr<MatchingCycleInterface> cycle)
{
    uint64_t tick{};
    while (true) {
        cycle->on_tick(tick++);
    }
}

} // namespace

int
main(int argc, const char** argv)
{
    nutc::logging::init(quill::LogLevel::Info);
    std::signal(SIGINT, [](auto) { std::exit(0); });
    std::signal(SIGPIPE, SIG_IGN);

    auto mode = process_arguments(argc, argv);
    TraderContainer traders{};
    AlgoInitializer::get_algo_initializer(mode)->initialize_algo_management(traders);

    main_event_loop(create_cycle(traders, mode));

    return 0;
}
