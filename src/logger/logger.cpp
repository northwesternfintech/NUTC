#include "logger.hpp" // includes fstream, string, optional

// GCC >= 13.2 supports std::chrono::utc_clock
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=104167
#if defined(__GNUC__) \
    && (__GNUC__ > 13 \
        || (__GNUC__ == 13 && __GNUC_MINOR__ >= 2))
#define FMT_USE_UTC_TIME 1
#endif

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <iostream>

namespace nutc {
namespace events {

void
Logger::log_event(
  MessageType type,
  const std::string& json_message,
  const std::optional<std::string>& uid
)
{
  // TODO: Need to fix this to have better formatting and more
  // useful information. Right now it spits out information.
  if (!output_file_.is_open()) [[unlikely]] {
    log_e(events, "Output file {} not open, unable to log event", get_file_name());
    return;
  }

  // Write start of JSON
  std::cout << "{ ";
  
  // Write current GMT time
  const auto now = std::chrono::utc_clock::now();
  std::cout << fmt::format("\"time\": \"{:%FT%TZ}\", ", now);

  // Add MessageType and JSON message (and opt UID) to file
  output_file_ << "\"type\": " << static_cast<int>(type) << ", "; // add type
  output_file_ << "\"message\": " << json_message; // add message
  
  if (uid.has_value()) {
    output_file_ << ", \"uid\": " << uid.value(); // add uid if exists
  }

  output_file_ << " }\n"; // close the brace and end the line
}

} // namespace events
} // namespace nutc
