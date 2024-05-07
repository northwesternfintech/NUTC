#include "wrapper/logging.hpp"

#include "wrapper/config.h"

#include <quill/Quill.h>

#define LOGLINE_FORMAT "%(logger_name:<8): %(message)"

namespace nutc {
namespace logging {

using namespace quill; // NOLINT(*-using-namespace)
using cc = quill::ConsoleColours;

void
init(const std::string& trader_id, quill::LogLevel log_level)
{
    detail::application_log_level = log_level;

#ifdef _WIN32
    // NOTE: on win32 a signal handler is needed for each new thread
    quill::init_signal_handler();
#endif

    // Create our config object
    quill::Config cfg;

    // Set main logger name
    cfg.default_logger_name = "main";

    //
    // Initialize print handler
    //
    quill::ConsoleColours colors;
    colors.set_colour(LogLevel::TraceL3, cc::white);
    colors.set_colour(LogLevel::TraceL2, cc::white);
    colors.set_colour(LogLevel::TraceL1, cc::white);
    colors.set_colour(LogLevel::Debug, cc::cyan);
    colors.set_colour(LogLevel::Info, cc::green);

#ifdef _WIN32
    // NOLINTBEGIN(*-signed-bitwise)
    colors.set_colour(LogLevel::Warning, cc::yellow | cc::bold);
    colors.set_colour(LogLevel::Error, cc::red | cc::bold);
    colors.set_colour(LogLevel::Critical, cc::bold | cc::white | cc::on_red);
    // NOLINTEND(*-signed-bitwise)
#else
    colors.set_colour(LogLevel::Warning, cc::yellow + cc::bold);
    colors.set_colour(LogLevel::Error, cc::red + cc::bold);
    colors.set_colour(LogLevel::Critical, cc::bold + cc::white + cc::on_red);
#endif

    colors.set_colour(LogLevel::Backtrace, cc::magenta);

    auto handler_cfg = quill::RotatingFileHandlerConfig{};
    handler_cfg.set_open_mode('w');
    handler_cfg.set_max_backup_files(0);
    handler_cfg.set_overwrite_rolled_files(true);
    handler_cfg.set_rotation_max_file_size(50000);

    auto file_handler = quill::rotating_file_handler("logs/" + trader_id, handler_cfg);

    file_handler->set_pattern(LOGLINE_FORMAT);
    file_handler->set_log_level(log_level);

    cfg.default_handlers.emplace_back(file_handler);

    // Send the config
    quill::configure(cfg);

    // Set backtrace and log level on the main logger
    quill::Logger* main_logger = quill::get_logger();
    main_logger->init_backtrace(LOG_BACKTRACE_SIZE, quill::LogLevel::Error);
    main_logger->set_log_level(log_level);

    // Set the thread name
    set_thread_name("MainThread");

    // Start the logging backend thread
    quill::start(true);

    LOG_INFO(main_logger, "Logging initialized!");
}

} // namespace logging
} // namespace nutc
