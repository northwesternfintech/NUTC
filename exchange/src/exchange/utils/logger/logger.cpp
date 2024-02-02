#include "exchange/utils/logger/logger.hpp" // includes fstream, string, optional

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <glaze/glaze.hpp>
#include <quill/Clock.h>

#include <chrono>

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
    using std::chrono::milliseconds, std::chrono::system_clock, std::chrono::time_point;

    // Get time from Quill
    auto now = quill::Clock::now();

    // Get epoch time in milliseconds, and create time point
    auto epoch_millis = duration_cast<milliseconds>(now.time_since_epoch());
    auto system_now = time_point<system_clock>{epoch_millis};

    static constexpr int HOURS = 23;

    return fmt::format("{:%Y-%m-%d %H:%M:%S}", system_now).substr(0, HOURS);
}

template <GlazeMetaSpecialized T>
void
Logger::log_event(const T& json_message)
{
    // If file is not open, throw an error to the error logger
    if (!output_file_.is_open()) [[unlikely]] {
        log_e(events, "Output file {} not open, unable to log event", get_file_name());
        return;
    }

    WithTimestamp<T> json_message_with_ts = {json_message};

    // TODO(andrlime): fix type
    std::string buffer = glz::write_json(json_message_with_ts);

    // Log to the main log too
    log_i(events, "Logging event {}", buffer);

    // Write start of JSON
    output_file_ << buffer << "\n";
    output_file_.flush();
}

// Explicit instantiations
template void Logger::log_event<messages::MarketOrder>(const messages::MarketOrder&);
template void Logger::log_event<messages::Match>(const messages::Match&);
template void Logger::log_event<messages::ObUpdate>(const messages::ObUpdate&);

} // namespace events
} // namespace nutc
