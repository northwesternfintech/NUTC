#pragma once

#include <boost/process.hpp>
#include <boost/process/pipe.hpp>

#include <atomic>

namespace nutc {
namespace wrappers {
namespace bp = boost::process;

class WrapperHandle {
    bp::child wrapper_;
    std::shared_ptr<bp::async_pipe> pipe_in;
    std::shared_ptr<bp::async_pipe> pipe_out;

    // TODO: make class to abstract this
    std::mutex messages_lock_{};
    std::deque<std::string> queued_messages_{};
    std::atomic_flag is_writing_{false};

    void write_out(std::string message);

public:
    WrapperHandle() = default;

    void
    terminate()
    {
        if (wrapper_.running())
            wrapper_.terminate();
    }

    void send_messages(std::vector<std::string> messages);

    void start(const std::string& path, const std::vector<std::string>& args);
    void stop();

    ~WrapperHandle() { terminate(); }
};
} // namespace wrappers
} // namespace nutc
