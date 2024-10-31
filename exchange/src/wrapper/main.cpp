#include "common/logging/logging.hpp"
#include "wrapper/config/argparse.hpp"
#include "wrapper/messaging/exchange_communicator.hpp"
#include "wrapper/runtime/cpp/cpp_runtime.hpp"
#include "wrapper/runtime/python/python_runtime.hpp"
#include "wrapper/startup/system.hpp"
#include "wrapper/util/resource_limits.hpp"

#include <boost/algorithm/string/trim.hpp>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

int
main(int argc, const char** argv)
{
    using namespace nutc::wrapper;

<<<<<<< Updated upstream
    std::signal(SIGINT, catch_sigint);
    std::signal(SIGTERM, catch_sigterm);
    auto [verbosity, trader_id, algo_type] = process_arguments(argc, argv);
=======
    auto [verbosity, trader_id, algo_type] = process_arguments(argc, argv);

    set_signal_handlers();
>>>>>>> Stashed changes

    static constexpr std::uint32_t MAX_LOG_SIZE = 50'000;
    nutc::logging::init_file_only(
        fmt::format("{}.log", trader_id), MAX_LOG_SIZE, quill::LogLevel::Info
    );

    set_memory_limit(1024);
    kill_on_exchange_death();

    nutc::common::algorithm_content algorithm =
        ExchangeCommunicator::consume_algorithm();

    if (algorithm.algorithm_content_str.empty()) {
        ExchangeCommunicator::report_startup_complete();
        return 1;
    }
    ExchangeCommunicator::report_startup_complete();
    ExchangeCommunicator::wait_for_start_time();

    if (algo_type == nutc::common::AlgoLanguage::python) {
        PyRuntime{algorithm.algorithm_content_str, trader_id}.main_event_loop();
    }
    else {
        CppRuntime{algorithm.algorithm_content_str, trader_id}.main_event_loop();
    }

    return 0;
}
