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

    void spawn_wrapper(const std::vector<std::string>& args);

public:
    // Remote (algo in firebase)
    WrapperHandle(const std::string& remote_uid, const std::string& algo_id);

    // Local (.py on disk)
    WrapperHandle(const std::string& algo_path);

    void send_messages(std::vector<std::string> messages);

    ~WrapperHandle()
    {
        if (wrapper_.running())
            wrapper_.terminate();
    }
};
} // namespace wrappers
} // namespace nutc
