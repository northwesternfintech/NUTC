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

    void add_message_(const std::string& message);

    void async_read_pipe(
        std::weak_ptr<bp::async_pipe> pipe_in_ptr, std::shared_ptr<std::string> buffer
    );

    void
    async_read_pipe(std::weak_ptr<bp::async_pipe> pipe_in_ptr)
    {
        async_read_pipe(pipe_in_ptr, std::make_shared<std::string>());
    }

public:
    PipeReader(std::weak_ptr<bp::async_pipe> pipe_in) { async_read_pipe(pipe_in); }

    std::vector<std::variant<messages::init_message, messages::market_order>>
    get_messages()
    {
        std::lock_guard<std::mutex> lock{message_lock_};
        auto ret = message_queue_;
        message_queue_.clear();
        return ret;
    }
};

} // namespace wrappers
} // namespace nutc
