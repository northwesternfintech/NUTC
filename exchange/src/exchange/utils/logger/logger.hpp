#pragma once

#include "exchange/config.h"
#include "exchange/logging.hpp"
#include "shared/messages.hpp" // TYPE should be an enum {AccountUpdate, OrderbookUpdate, TradeUpdate, MarketOrder}

#include <fstream>
#include <optional>
#include <string>

namespace nutc {
namespace events {

std::string timestamp_in_ms();

template <typename T>
concept GlazeMetaSpecialized = requires {
    {
        glz::meta<T>::value
    } -> std::convertible_to<decltype(glz::meta<T>::value)>;
};

template <GlazeMetaSpecialized T>
struct WithTimestamp {
    const T& data;
    std::string timestamp;

    // Constructor to initialize the base data and timestamp
    WithTimestamp(const T& data) :
        data(data), timestamp(nutc::events::timestamp_in_ms())
    {}
};

enum class MESSAGE_TYPE { // needs to be changed to something better, but I will leave
                          // this here for now
    MARKET_ORDER,
    MATCH
};

class Logger {
    /**
     * @brief The file name to log events to
     */
    std::string file_name_;

    /**
     * @brief The output file object
     */
    std::ofstream output_file_;

public:
    /**
     * @brief Construct a new Logger object
     *
     * @param file_name File name to log to
     */

    static Logger& get_logger();

    // Logger(const Logger&) = delete;
    // Logger(Logger&&) = delete;
    // Logger& operator=(const Logger&) = delete;
    // Logger& operator=(Logger&&) = delete;

    /**
     * @brief Log an event to this Logger's file
     *
     * @param json_message the message to log
     * @param uid optional UID to log with this message
     */
    template <GlazeMetaSpecialized T>
    void log_event(const T& json_message);

    /**
     * @brief Get the file name string
     *
     * @return std::string
     */
    [[nodiscard]] const std::string&
    get_file_name() const
    {
        return file_name_;
    }

private:
    explicit Logger(std::string file_name) :
        file_name_(file_name), output_file_(file_name, std::ios::out | std::ios::app)
    {}
};

} // namespace events
} // namespace nutc

template <nutc::events::GlazeMetaSpecialized T>
struct glz::meta<nutc::events::WithTimestamp<T>> {
    using U = nutc::events::WithTimestamp<T>;
    /* clang-format off */
    static constexpr auto value = object(
        "data", [](auto&& self) -> auto& { return self.data; }, // Serialize orig. data
        "timestamp", [](auto&& self) -> auto& { return self.timestamp; } // Timestamp
    );
    /* clang-format on */
};
