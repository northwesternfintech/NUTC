#include "algos/algo_manager.hpp"
#include "common/logging/logging.hpp"
#include "common/util.hpp"
#include "exchange/algos/algo_manager.hpp"
#include "exchange/config/dynamic/argparse.hpp"
#include "exchange/config/dynamic/config.hpp"
#include "exchange/exchange_state.hpp"
#include "exchange/matching_cycle/base/base_cycle.hpp"
#include "exchange/matching_cycle/cycle_interface.hpp"
#include "exchange/matching_cycle/dev/dev_cycle.hpp"
#include "exchange/matching_cycle/sandbox/sandbox_cycle.hpp"

#include <csignal>

#include <utility>

namespace {
using namespace nutc::exchange; // NOLINT

std::unique_ptr<MatchingCycleInterface>
create_cycle(exchange_state& state, const auto& mode)
{
    using nutc::common::Mode;
    double order_fee = Config::get().constants().ORDER_FEE;
    auto max_order_volume = Config::get().constants().MAX_CUMULATIVE_OPEN_ORDER_VOLUME;

    switch (mode) {
        case Mode::normal:
            return std::make_unique<BaseMatchingCycle>(
                state, order_fee, max_order_volume
            );
        case Mode::sandbox:
            return std::make_unique<SandboxMatchingCycle>(
                state, order_fee, max_order_volume
            );
        case Mode::bots_only:
        case Mode::dev:
            return std::make_unique<DevMatchingCycle>(
                state, order_fee, max_order_volume
            );
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
    nutc::common::logging_init("exchange.log", quill::LogLevel::Info);
    std::signal(SIGINT, [](auto) { std::exit(0); });
    std::signal(SIGPIPE, SIG_IGN);

    auto mode = process_arguments(argc, argv);
    const auto& ticker_config = Config::get().get_tickers();
    exchange_state state{ticker_config};

    AlgoInitializer::get_algo_initializer(mode)->initialize_algo_management(
        state.traders
    );

    main_event_loop(create_cycle(state, mode));

    return 0;
}
