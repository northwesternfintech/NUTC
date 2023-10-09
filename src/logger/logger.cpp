#include <iostream>
#include "logger.hpp" // includes fstream, string, optional

namespace nutc {
namespace events {
namespace logger_class {

void
Logger::log_event(
  MessageType type,
  const std::string& json_message,
  const std::optional<std::string>& uid
)
{
  // TODO: Need to fix this to have better formatting and more
  // useful information. Right now it spits out information.
  bool output_file_closed = !output_file.is_open();
  if (output_file_closed) {
    log_e(events, "Output file {} not open, unable to log event", get_file_name());
    return;
  }
  
  // Write MessageType, JSON message, and optional UID to file
  output_file << "{ \"type\": " << static_cast<int>(type) << ", "; // add type
  output_file << "\"message\": " << json_message; // add message
  
  if (uid.has_value()) {
    output_file << ", \"uid\": " << uid.value(); // add uid if exists
  }

  output_file << " }\n"; // close the brace and end the line
}

} // namespace logger_class
} // namespace events
} // namespace nutc
