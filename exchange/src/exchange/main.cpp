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
using namespace nutc; // NOLINT

matching::TickerMapping
load_tickers(traders::TraderContainer& traders)
{
    matching::TickerMapping ret;
    const auto& tickers = config::Config::get().get_tickers();
    for (const config::ticker_config& ticker : tickers) {
        ret.insert({
            ticker.TICKER, {traders, ticker}
        });
    }
    return ret;
}

std::unique_ptr<matching::MatchingCycleInterface>
create_cycle(traders::TraderContainer& traders, const auto& mode)
{
    using util::Mode;
    auto tickers = load_tickers(traders);

    switch (mode) {
        case Mode::normal:
            return std::make_unique<matching::BaseMatchingCycle>(tickers, traders);
        case Mode::sandbox:
            return std::make_unique<matching::SandboxMatchingCycle>(tickers, traders);
        case Mode::bots_only:
        case Mode::dev:
            return std::make_unique<matching::DevMatchingCycle>(tickers, traders);
    }

    std::unreachable();
}

void
main_event_loop(std::unique_ptr<matching::MatchingCycleInterface> cycle)
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
    logging::init(quill::LogLevel::Info);
    std::signal(SIGINT, [](auto) { std::exit(0); });
    std::signal(SIGPIPE, SIG_IGN);

    auto mode = config::process_arguments(argc, argv);
    traders::TraderContainer traders{};
    algos::AlgoInitializer::get_algo_initializer(mode)->initialize_algo_management(
        traders
    );

    main_event_loop(create_cycle(traders, mode));

    return 0;
}
