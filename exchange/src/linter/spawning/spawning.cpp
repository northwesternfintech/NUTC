#include "spawning.hpp"

#include "async_read_with_timeout.hpp"
#include "common/util.hpp"
#include "linter/config.h"

#include <fmt/core.h>
#include <glaze/json/read.hpp>

#include <filesystem>

namespace nutc {
namespace linter {

namespace {
namespace bp = boost::process;
namespace ba = boost::asio;

struct algorithm_process {
    std::unique_ptr<bp::child> process;
    std::unique_ptr<bp::async_pipe> pipe_algorithm_to_linter;
};

static algorithm_process spawn_algorithm(
    const std::string& algo_code, common::AlgoLanguage language,
    ba::io_context& io_context
);

static const std::filesystem::path& spawner_binary_path();

static std::string get_language_flag(common::AlgoLanguage language);

const std::filesystem::path&
spawner_binary_path()
{
    static constexpr auto LINTER_SPAWNER_BINARY_ENV_VAR =
        "NUTC_LINTER_SPAWNER_BINARY_PATH";
    static const char* spawner_binary_location =
        std::getenv(LINTER_SPAWNER_BINARY_ENV_VAR); // NOLINT
    if (spawner_binary_location == nullptr) [[unlikely]] {
        throw std::runtime_error(fmt::format(
            "{} environment variable not set", LINTER_SPAWNER_BINARY_ENV_VAR
        ));
    }

    static const std::filesystem::path spawner_binary_path{spawner_binary_location};
    if (!std::filesystem::exists(spawner_binary_path)) {
        throw std::runtime_error(
            fmt::format("File at {} does not exist", LINTER_SPAWNER_BINARY_ENV_VAR)
        );
    }

    return spawner_binary_path;
}

std::string
get_language_flag(common::AlgoLanguage language)
{
    switch (language) {
        case common::AlgoLanguage::python:
            return "-python";
        case common::AlgoLanguage::cpp:
            return "-cpp";
        default:
            throw std::runtime_error("Unknown language");
    }
}

algorithm_process
spawn_algorithm(
    const std::string& algo_code, common::AlgoLanguage language,
    ba::io_context& io_context
)
{
    const static std::string spawner_path = spawner_binary_path();

    auto algo_to_linter_pipe = std::make_unique<bp::async_pipe>(io_context);

    bp::opstream linter_to_algo_pipe;
    auto algorithm_process = std::make_unique<bp::child>(
        bp::exe(spawner_path), bp::args(get_language_flag(language)),
        bp::std_in<linter_to_algo_pipe, bp::std_err> stderr,
        bp::std_out > *algo_to_linter_pipe, io_context
    );

    if (!algorithm_process->valid()) {
        throw std::runtime_error("Failed to create linter spawner process");
    }

    linter_to_algo_pipe << common::base64_encode(algo_code) << std::endl;
    linter_to_algo_pipe.pipe().close();
    return {std::move(algorithm_process), std::move(algo_to_linter_pipe)};
}
} // namespace

lint_result
spawn_client(const std::string& algo_code, common::AlgoLanguage language)
{
    boost::asio::io_context io_context;
    auto [process, pipe_from_algo] = spawn_algorithm(algo_code, language, io_context);
    lint_result res;
    ba::streambuf buffer;

    auto async_read = std::make_unique<AsyncReadWithTimeout>(
        io_context, *pipe_from_algo, buffer,
        std::chrono::seconds{LINT_AUTO_TIMEOUT_SECONDS},
        [&buffer, &res, &process,
         &pipe_from_algo](const boost::system::error_code& err, std::size_t) {
            if (!err) {
                std::istream stream(&buffer);
                std::string message;
                std::getline(stream, message);
                std::string decoded_message = common::base64_decode(message);
                auto error = glz::read_json<lint_result>(res, decoded_message);
                if (error) {
                    res = {
                        false, "Internal server error. Reach out to "
                               "nuft@u.northwesten.edu for support"
                    };
                }
            }
            else if (err == boost::asio::error::operation_aborted) {
                res.success = false;
                res.message += fmt::format(
                    "[linter] FAILED to lint algo\n\nYour code did not execute within "
                    "{} "
                    "seconds. Check for infinite loops or recursion. If the issue "
                    "persists, "
                    "reach out on Piazza.\n",
                    LINT_AUTO_TIMEOUT_SECONDS
                );
            }
            else {
                res = {
                    false, "Internal server error. Reach out to nuft@u.northwesten.edu "
                           "for support"
                };
            }

            if (process->running()) {
                process->terminate();
            }
            pipe_from_algo->cancel();
        }
    );

    io_context.run();

    return res;
}

} // namespace linter
} // namespace nutc
