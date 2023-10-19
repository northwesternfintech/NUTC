#include "logger.hpp" // includes fstream, string, optional

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>

#include <iostream>

namespace nutc {
namespace events {

Logger&
Logger::get_logger(const std::string& file_name)
{
    static std::unordered_map<std::string, Logger> loggers;
    auto it = loggers.find(file_name);
    if (it == loggers.end()) {
        it = loggers.emplace(file_name, Logger(file_name)).first;
    }
    return it->second;
}

void
Logger::log_event(
    MESSAGE_TYPE type, const std::string& json_message,
    const std::optional<std::string>& uid
)
{
    // If file is not open, throw an error to the error logger
    if (!output_file_.is_open()) [[unlikely]] {
        log_e(events, "Output file {} not open, unable to log event", get_file_name());
        return;
    }

    // Write start of JSON
    output_file_ << "{ ";

    // Write current GMT time
    const auto now = std::chrono::system_clock::now();
    output_file_ << fmt::format("\"time\": \"{:%FT%TZ}\", ", now);

    // Add MessageType and JSON message (and opt UID) to file
    output_file_ << "\"type\": " << static_cast<int>(type) << ", "; // add type
    output_file_ << "\"message\": " << json_message;                // add message

    if (uid.has_value()) {
        output_file_ << ", \"uid\": " << uid.value(); // add uid if exists
    }

    output_file_ << " }\n"; // close the brace and end the line
}

} // namespace events
} // namespace nutc
