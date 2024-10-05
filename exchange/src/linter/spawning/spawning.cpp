#include "spawning.hpp"

#include "common/util.hpp"
#include "linter/config.h"

#include <fmt/core.h>
#include <glaze/json/read.hpp>

namespace bp = boost::process;

namespace nutc {
namespace spawning {

const std::filesystem::path&
LintProcessManager::spawner_binary_path()
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

nutc::lint::lint_result
LintProcessManager::spawn_client(const std::string& algo_code, AlgoLanguage language)
{
    static const std::string path{spawner_binary_path()};
    auto in_pipe = std::make_shared<bp::async_pipe>(io_context);
    bp::opstream out_pipe;

    auto get_language_flag = [](AlgoLanguage language) {
        switch (language) {
            case nutc::spawning::AlgoLanguage::Python:
                return "-python";
            case nutc::spawning::AlgoLanguage::Cpp:
                return "-cpp";
            default:
                throw std::runtime_error("Unknown language");
        }
    };

    auto child = std::make_shared<bp::child>(
        bp::exe(path), bp::args(get_language_flag(language)),
        bp::std_in<out_pipe, bp::std_err> stderr, bp::std_out > *in_pipe, io_context
    );

    out_pipe << common::base64_encode(algo_code) << std::endl;
    out_pipe.pipe().close();

    auto kill_timer = std::make_shared<ba::steady_timer>(io_context);
    kill_timer->expires_after(ba::chrono::seconds(LINT_AUTO_TIMEOUT_SECONDS));

    nutc::lint::lint_result res;

    kill_timer->async_wait([child, in_pipe, &res](const auto& ec) {
        if (!ec) {
            in_pipe->close();
            res.success = false;
            res.message += fmt::format(
                "[linter] FAILED to lint algo\n\nYour code did not execute within "
                "{} "
                "seconds. Check all "
                "functions to see if you have an infinite loop or infinite "
                "recursion.\n\nIf you continue to experience this error, "
                "reach out to #nuft-support.\n",
                LINT_AUTO_TIMEOUT_SECONDS
            );

            if (child->running()) {
                child->terminate();
            }
        }
    });

    auto buffer = std::make_shared<ba::streambuf>();
    async_read_until(
        *in_pipe, *buffer, '\n',
        [buffer, kill_timer, child, in_pipe, &res](const auto& ec, auto) {
            if (!ec) {
                kill_timer->cancel();

                std::istream stream(buffer.get());
                std::string line;
                std::string message;

                while (std::getline(stream, line)) {
                    message += line;
                }
                if (child->running()) {
                    child->terminate();
                }
                std::string decoded_message = common::base64_decode(message);

                auto error =
                    glz::read_json<nutc::lint::lint_result>(res, decoded_message);
                if (error) {
                    res = {
                        false, "Internal server error. Reach out to "
                               "nuft@u.northwesten.edu "
                               "for support"
                    };
                };
                in_pipe->cancel();
            }
        }
    );

    io_context.run();
    io_context.restart();

    return res;
}

} // namespace spawning
} // namespace nutc
