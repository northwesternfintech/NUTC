#include "argparse.hpp"

#include "common/config/config.h"

#include <argparse/argparse.hpp>
#include <fmt/format.h>

#include <string>

namespace nutc::wrapper {
wrapper_args
process_arguments(int argc, const char** argv)
{
    argparse::ArgumentParser program(
        "NUTC Wrapper", NUTC_VERSION, argparse::default_arguments::help
    );

    program.add_argument("-U", "--uid")
        .help("set the user ID")
        .action([](const auto& value) {
            std::string uid = std::string(value);
            std::replace(uid.begin(), uid.end(), ' ', '-');
            return uid;
        })
        .required();

    auto& language_group = program.add_mutually_exclusive_group(true);
    language_group.add_argument("-B", "--cpp_algo")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);
    language_group.add_argument("-P", "--python_algo")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    uint8_t verbosity = 0;
    program.add_argument("-v", "--verbose")
        .help("increase output verbosity")
        .action([&](const auto& /* unused */) { ++verbosity; })
        .append()
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("--core_num").nargs(1).required();

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl; // NOLINT
        std::cerr << program;
        exit(1); // NOLINT(concurrency-*)
    }

    auto trader_id = program.get<std::string>("--uid");
    int core_num = std::stoi(program.get<std::string>("--core_num"));

    if (program.get<bool>("--cpp_algo")) {
        return {verbosity, trader_id, common::AlgoLanguage::cpp, core_num};
    }
    if (program.get<bool>("--python_algo")) {
        return {verbosity, trader_id, common::AlgoLanguage::python, core_num};
    }

    throw std::runtime_error("No language provided");
}
} // namespace nutc::wrapper
