#pragma once

#include <string>
#include <fstream>
#include <optional>

#include "util/messages.hpp" // TYPE should be an enum {AccountUpdate, OrderbookUpdate, TradeUpdate, MarketOrder}
#include "logging.hpp"

namespace nutc {
namespace logger_class {

  enum class MessageType { // needs to be changed to something better, but I will leave this here for now
    ACCOUNT_UPDATE,
    ORDERBOOK_UPDATE,
    TRADE_UPDATE,
    MARKET_ORDER
  };

  class Logger {

    private:
      /**
       * @brief The file name to log events to
       */
      std::string file_name;

      /**
       * @brief The output file object
       */
      std::ofstream output_file;

    public:
      /**
       * @brief Log an event to this Logger's file
       * 
       * @param type the type of message this is, see `enum MessageType`
       * @param json_message the message to log
       * @param uid optional UID to log with this message
       */
      void log_event(MessageType type, const std::string& json_message, const std::optional<std::string>& uid);

      /**
       * @brief Construct a new Logger object
       * 
       * @param file_name File name to log to
       */
      explicit Logger(
        std::string file_name
      ) : file_name(file_name)
      {
        this->output_file.open(this->file_name, std::ios::out | std::ios::app); // open the logging file
      }

      /**
       * @brief Get the file name string
       * 
       * @return std::string 
       */
      [[nodiscard]] const std::string& get_file_name() const
      {
        return file_name;
      }

  };

} // namespace logger_class
} // namespace nutc
