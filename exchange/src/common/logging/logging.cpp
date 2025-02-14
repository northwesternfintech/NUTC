#include "common/logging/logging.hpp"

#include <quill/Quill.h>

#include <string>

#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)                \
    || defined(QUILL_NO_THREAD_NAME_SUPPORT)
#  define LOGLINE_FORMAT                                                               \
      "%(ascii_time) [%(thread:<6)] [%(logger_name:<8)] %(level_name:<8) - "           \
      "%(message) (%(fileline))"
#else
#  define LOGLINE_FORMAT                                                               \
      "%(ascii_time) [%(thread_name:<12)] [%(logger_name:<8)] %(level_name:<8) - "     \
      "%(message) (%(fileline))"
#endif

#ifdef _WIN32
#  define TZ_FORMAT ""
#else
#  define TZ_FORMAT " %z"
#endif

#define SHORT_LOGLINE_FORMAT "%(logger_name:<8): %(message)"

namespace nutc {
namespace common {

using namespace quill; // NOLINT(*-using-namespace)
using cc = quill::ConsoleColours;

void
logging_init(const std::string& log_file, quill::LogLevel log_level)
{
    detail::application_log_level = log_level;

    // Create the logs directory
    if (!std::filesystem::is_directory(LOG_DIR))
        std::filesystem::create_directory(LOG_DIR);

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

    auto stdout_handler = quill::stdout_handler("console", colors);
    stdout_handler->set_pattern(
        LOGLINE_FORMAT,
        "%Y-%m-%dT%T.%Qms" TZ_FORMAT // ISO 8601 but with space instead of T
    );
    stdout_handler->set_log_level(log_level);

    cfg.default_handlers.emplace_back(stdout_handler);

    auto handler_cfg = quill::FileHandlerConfig{};
    handler_cfg.set_open_mode('w');

    std::string full_log_path = fmt::format("{}/{}", LOG_DIR, log_file);
    auto file_handler = quill::file_handler(full_log_path, handler_cfg);

    file_handler->set_pattern(
        LOGLINE_FORMAT,
        "%Y-%m-%dT%T.%Qms" TZ_FORMAT // ISO 8601
    );
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
    quill::start();

    LOG_INFO(main_logger, "Logging initialized!");
}

void
init_file_only(
    const std::string& log_file, std::uint32_t max_size_bytes, quill::LogLevel log_level
)
{
    detail::application_log_level = log_level;

    // Create the logs directory
    if (!std::filesystem::is_directory(LOG_DIR))
        std::filesystem::create_directory(LOG_DIR);

    // Create our config object
    quill::Config cfg;

    // Set main logger name
    cfg.default_logger_name = "main";

    auto handler_cfg = quill::RotatingFileHandlerConfig{};
    handler_cfg.set_open_mode('w');
    handler_cfg.set_max_backup_files(0);
    handler_cfg.set_overwrite_rolled_files(true);
    handler_cfg.set_rotation_max_file_size(max_size_bytes);

    std::string full_log_path = fmt::format("{}/{}", LOG_DIR, log_file);
    auto file_handler = quill::rotating_file_handler(full_log_path, handler_cfg);

    file_handler->set_pattern(SHORT_LOGLINE_FORMAT);
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
    quill::start();

    LOG_INFO(main_logger, "Logging initialized!");
}

} // namespace common
} // namespace nutc
