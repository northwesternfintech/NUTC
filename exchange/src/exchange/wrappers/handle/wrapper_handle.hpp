#pragma once

#include <boost/process.hpp>
#include <boost/process/pipe.hpp>

#include <atomic>
#include <filesystem>

namespace nutc {
namespace wrappers {
namespace bp = boost::process;
namespace fs = std::filesystem;

class WrapperHandle {
    bp::child wrapper_;
    std::weak_ptr<bp::async_pipe> pipe_in;
    std::weak_ptr<bp::async_pipe> pipe_out;
    std::mutex messages_lock_{};
    std::deque<std::string> queued_messages_{};
    std::atomic_flag is_writing_{false};

    void async_write_pipe(std::string message);

    void spawn_wrapper(const std::vector<std::string>& args);

    const fs::path& wrapper_binary_path();

public:
    // Remote (algo in firebase)
    WrapperHandle(const std::string& remote_uid, const std::string& algo_id);

    // Local (.py on disk)
    WrapperHandle(const std::string& algo_path);

    void send_messages(std::vector<std::string> messages);

    ~WrapperHandle();
};
} // namespace wrappers
} // namespace nutc
