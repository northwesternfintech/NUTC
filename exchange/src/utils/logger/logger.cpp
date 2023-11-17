#include "utils/logger/logger.hpp" // includes fstream, string, optional

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <glaze/glaze.hpp>

#include <chrono>

#include <iostream>

namespace nutc {
namespace events {

Logger&
Logger::get_logger()
{
    static Logger logger(JSON_LOG_FILE);
    return logger;
}

void
Logger::log_event(MESSAGE_TYPE type, const glz::json_t& json_message)
{
    // If file is not open, throw an error to the error logger
    if (!output_file_.is_open()) [[unlikely]] {
        log_e(events, "Output file {} not open, unable to log event", get_file_name());
        return;
    }

    // TODO: fix type
    std::string buffer = glz::write_json(json_message);

    // Write start of JSON
    output_file_ << buffer << "\n";
}

} // namespace events
} // namespace nutc
