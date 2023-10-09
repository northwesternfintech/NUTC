#include "logger.hpp" // includes fstream, string, optional

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
  
  // Write current GMT time
  const auto now = std::chrono::system_clock::now(); // get current time
  std::time_t now_t = std::chrono::system_clock::to_time_t(now); // convert to a time_t type
  std::tm* now_tm = std::gmtime(&now_t); // convert to GMT time
  output_file_ << "{ \"time\": \"" << fmt::format("{:%Y-%m-%dT%H:%M:%S}Z", *now_tm) << "\", "; // TODO: I do not know if this works.

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
