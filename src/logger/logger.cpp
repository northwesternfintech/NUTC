#include "engine.hpp" // includes fstream, string, optional

#include <iostream>
#include "logger.hpp"

namespace nutc {
namespace logger_class {

void
Logger::log_event(
  MessageType type,
  std::string json_message,
  std::optional<std::string> uid
)
{
  
}

Logger::Logger(
  std::string file_name
) : file_name(file_name)
{
  this->output_file.open(this->file_name, std::ios::out | std::ios::app); // open the logging file
}

Logger::~Logger()
{
  this->output_file.close(); // close the logging file
}

} // namespace logger_class
} // namespace nutc
