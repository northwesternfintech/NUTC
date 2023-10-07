#pragma once

#include <string>
#include <fstream>
#include <optional>

#include "util/messages.hpp" // TYPE should be an enum {AccountUpdate, OrderbookUpdate, TradeUpdate, MarketOrder}

namespace nutc {
namespace logger_class {

  enum MessageType { // needs to be changed to something better, but I will leave this here for now
    AccountUpdate,
    ObUpdate,
    TradeUpdate,
    MarketOrder
  };

  class Logger {

    public:
      /**
       * @brief Log an event to this Logger's file
       * 
       * @param type the type of message this is, see `enum MessageType`
       * @param json_message the message to log
       * @param uid optional UID to log with this message
       */
      void log_event(MessageType type, std::string json_message, std::optional<std::string> uid);

      /**
       * @brief Construct a new Logger object
       * 
       * @param file_name File name to log to
       */
      Logger(std::string file_name);

      /**
       * @brief Destroy the Logger object and close the output file
       */
      ~Logger();

      /**
       * @brief Get the file name string
       * 
       * @return std::string 
       */
      std::string get_file_name();

    private:
      /**
       * @brief The file name to log events to
       */
      std::string file_name;

      /**
       * @brief The output file object
       */
      std::ofstream output_file;

  };

} // namespace logger_class
} // namespace nutc
