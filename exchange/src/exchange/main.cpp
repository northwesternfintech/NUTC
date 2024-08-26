#include "algos/algo_manager.hpp"
#include "exchange/algos/algo_manager.hpp"
#include "exchange/config/dynamic/argparse.hpp"
#include "exchange/config/dynamic/config.hpp"
#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/logging.hpp"
#include "exchange/matching_cycle/base/base_cycle.hpp"
#include "exchange/matching_cycle/cycle_interface.hpp"
#include "exchange/matching_cycle/dev/dev_cycle.hpp"
#include "exchange/matching_cycle/sandbox/sandbox_cycle.hpp"
#include "exchange/orders/ticker_info.hpp"
#include "exchange/traders/trader_container.hpp"
#include "shared/util.hpp"

#include <csignal>

#include <utility>

namespace {
using namespace nutc::exchange; // NOLINT

TickerMapping
load_tickers(TraderContainer& traders, double order_fee)
{
    TickerMapping ret;
    const auto& tickers = Config::get().get_tickers();
    for (const ticker_config& ticker : tickers) {
        ret.insert({
            ticker.TICKER, {traders, ticker, order_fee}
        });
    }
    return ret;
}

std::unique_ptr<MatchingCycleInterface>
create_cycle(TraderContainer& traders, double order_fee, const auto& mode)
{
    using nutc::shared::Mode;
    auto tickers = load_tickers(traders, order_fee);

    switch (mode) {
        case Mode::normal:
            return std::make_unique<BaseMatchingCycle>(tickers, traders);
        case Mode::sandbox:
            return std::make_unique<SandboxMatchingCycle>(tickers, traders);
        case Mode::bots_only:
        case Mode::dev:
            return std::make_unique<DevMatchingCycle>(tickers, traders);
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

    double order_fee = Config::get().constants().ORDER_FEE;
    main_event_loop(create_cycle(traders, order_fee, mode));

    return 0;
}
