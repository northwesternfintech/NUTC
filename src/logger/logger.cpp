#include "logger.hpp" // includes fstream, string, optional

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
      // TODO: Need to fix this to have better formatting and more
      // useful information. Right now it spits out information.
      if (output_file.is_open()) {
        // Write MessageType, JSON message, and optional UID to file
        output_file << "Type: " << static_cast<int>(type) << ", ";
        output_file << "Message: " << json_message;
        
        if (uid.has_value()) {
          output_file << ", UID: " << uid.value();
        }

        output_file << std::endl;
      } else {
        std::cerr << "File " << this->get_file_name() << " not open. Unable to log event." << std::endl;
      }
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

    std::string Logger::get_file_name()
    {
      return this->file_name;
    }

  } // namespace logger_class
} // namespace nutc
