#pragma once

#include <boost/process.hpp>

#include <deque>
#include <mutex>

namespace nutc {
namespace wrappers {
namespace bp = boost::process;

class PipeWriter {
    std::mutex message_lock_{};
    std::deque<std::string> queued_messages_{};
    std::atomic_flag is_writing_{false};
    std::weak_ptr<bp::async_pipe> pipe_out_ptr;

    // Does not necessarily continue until pipe closed/canceled
    // This is as opposed to async_read_pipe
    void async_write_pipe(std::string message);

public:
    PipeWriter();
    ~PipeWriter();

    std::weak_ptr<bp::async_pipe>
    get_pipe()
    {
        return pipe_out_ptr;
    }

    void send_messages(std::vector<std::string> messages);
};

} // namespace wrappers
} // namespace nutc
