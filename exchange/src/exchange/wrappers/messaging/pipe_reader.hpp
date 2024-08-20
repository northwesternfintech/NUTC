#pragma once
#include "shared/messages_wrapper_to_exchange.hpp"

#include <boost/process.hpp>

#include <mutex>

namespace nutc {
namespace wrappers {

using init_message = messages::init_message;
using limit_order = messages::limit_order;
using market_order = messages::market_order;

namespace bp = boost::process;
namespace ba = boost::asio;

class PipeReader {
    using ReadMessageVariant = std::variant<init_message, limit_order, market_order>;

    std::mutex message_lock_{};
    std::vector<ReadMessageVariant> message_queue_{};
    std::shared_ptr<ba::io_context> pipe_context_;
    bp::async_pipe pipe_in_;

    void store_message_(const std::string& message);

    // Continues until pipe closed/canceled, so we can use RAII
    void async_read_pipe();
    void async_read_pipe(std::shared_ptr<std::string> buffer);

public:
    bp::async_pipe&
    get_pipe()
    {
        return pipe_in_;
    }

    PipeReader();

    ~PipeReader();

    // Nonblocking, all available messages
    std::vector<ReadMessageVariant> get_messages();

    // Blocking, O(messages) due to erase
    // Use sparingly
    ReadMessageVariant get_message();
};

} // namespace wrappers
} // namespace nutc
