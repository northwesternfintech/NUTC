#pragma once

#include <boost/process.hpp>

#include <deque>
#include <mutex>

namespace nutc::exchange {

namespace bp = boost::process;
namespace ba = boost::asio;

class PipeWriter {
    std::mutex message_lock_{};
    std::deque<std::string> queued_shared_{};
    std::atomic_flag is_writing_{false};
    std::shared_ptr<ba::io_context> pipe_context_;
    bp::async_pipe pipe_out_;

    // Does not necessarily continue until pipe closed/canceled
    // This is as opposed to async_read_pipe
    void async_write_pipe(std::string message);

public:
    PipeWriter();
    ~PipeWriter();

    bp::async_pipe&
    get_pipe()
    {
        return pipe_out_;
    }

    void send_message(const std::string& message);
};

} // namespace nutc::exchange
