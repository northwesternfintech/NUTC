#include "wrapper/algo_wrapper/binary/binary_wrapper.hpp"
#include "wrapper/algo_wrapper/python/python_wrapper.hpp"
#include "wrapper/algo_wrapper/wrapper.hpp"
#include "wrapper/config/argparse.hpp"
#include "wrapper/messaging/exchange_communicator.hpp"
#include "wrapper/util/logging.hpp"
#include "wrapper/util/resource_limits.hpp"

#include <boost/algorithm/string/trim.hpp>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#include <csignal>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// We stop the exchange with sigint. The wrapper should exit gracefully
void
catch_sigint(int)
{
    // Wait until we're forced to terminate
    while (true) {}
}

int
main(int argc, const char** argv)
{
    using namespace nutc::wrapper;

    std::signal(SIGINT, catch_sigint);
    auto [verbosity, trader_id, algo_type] = process_arguments(argc, argv);

    ExchangeCommunicator communicator{trader_id};

    if (!set_memory_limit(1024) || !kill_on_exchange_death()) {
        log_e(main, "Failed to set memory limit");
        communicator.report_startup_complete();
        return 1;
    }

    nutc::common::algorithm_content algorithm = communicator.consume_algorithm();

    if (algorithm.algorithm_content_str.empty()) {
        communicator.report_startup_complete();
        return 1;
    }
    communicator.report_startup_complete();
    communicator.wait_for_start_time();

    std::unique_ptr<Wrapper> wrapper;

    if (algo_type == nutc::common::AlgoType::python) {
        wrapper = std::make_unique<PyWrapper>(
            algorithm.algorithm_content_str, trader_id, communicator
        );
    }
    else {
        wrapper = std::make_unique<BinaryWrapper>(
            algorithm.algorithm_content_str, trader_id, communicator
        );
        // std::cerr << "binary_algo!!\n";
        // throw std::runtime_error("hello from cpp land!");
    }

    wrapper->main_event_loop();

    // communicator.main_event_loop();
    return 0;
}
