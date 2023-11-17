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

std::string
timestamp_in_ms()
{
    using namespace std::chrono;

    auto now = system_clock::now();

    auto nowAsTimeT = system_clock::to_time_t(now);

    auto nowAsTm = *std::localtime(&nowAsTimeT);

    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // Use stringstream to format the time
    std::ostringstream timestamp;
    timestamp << std::put_time(&nowAsTm, "%Y-%m-%d %H:%M:%S");
    timestamp << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return timestamp.str();
}

template <MetaSpecialized T>
void
Logger::log_event(const T& json_message)
{
    // If file is not open, throw an error to the error logger
    if (!output_file_.is_open()) [[unlikely]] {
        log_e(events, "Output file {} not open, unable to log event", get_file_name());
        return;
    }

    WithTimestamp<T> json_message_with_ts = {json_message};

    // TODO: fix type
    std::string buffer = glz::write_json(json_message_with_ts);

    // Write start of JSON
    output_file_ << buffer << "\n";
}

// Explicit instantiations
template void
Logger::log_event<messages::MarketOrder>(const messages::MarketOrder& json_message);
template void Logger::log_event<messages::Match>(const messages::Match& json_message);

} // namespace events
} // namespace nutc
