#pragma once
#include "shared/messages_wrapper_to_exchange.hpp"

#include <boost/process.hpp>

#include <mutex>

namespace nutc {
namespace wrappers {

using init_message = messages::init_message;
using market_order = messages::market_order;

namespace bp = boost::process;

/**
 * Singleton that stores incoming messages from wrappers
 */
class PipeReader {
    std::mutex message_lock_{};
    std::vector<std::variant<init_message, messages::market_order>> message_queue_{};
    std::weak_ptr<bp::async_pipe> pipe_in_ptr;

    void store_message_(const std::string& message);

    // Continues until pipe closed/canceled, so we can use RAII
    void async_read_pipe();
    void async_read_pipe(std::shared_ptr<std::string> buffer);

public:
    std::weak_ptr<bp::async_pipe>
    get_pipe()
    {
        return pipe_in_ptr;
    }

    PipeReader();

    ~PipeReader();

    std::vector<std::variant<messages::init_message, messages::market_order>>
    get_messages();
};

} // namespace wrappers
} // namespace nutc
