#pragma once
#include "message_storage.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

#include <boost/process.hpp>

#include <mutex>

namespace nutc {
namespace wrappers {

using namespace messages;

namespace bp = boost::process;
namespace ba = boost::asio;

class PipeReader {
    std::mutex message_lock_;
    MessageStorage<timed_init_message, timed_limit_order, timed_market_order>
        message_storage_;
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
    PipeReader(const PipeReader&) = delete;
    PipeReader(PipeReader&&) = delete;
    PipeReader& operator=(const PipeReader&) = delete;
    PipeReader& operator=(PipeReader&&) = delete;
    ~PipeReader();

    // Nonblocking, all available messages
    template <typename MessageT>
    std::vector<MessageT>
    get_messages()
    {
        std::lock_guard<std::mutex> lock{message_lock_};
        return message_storage_.extract<MessageT>();
    }
};

} // namespace wrappers
} // namespace nutc
