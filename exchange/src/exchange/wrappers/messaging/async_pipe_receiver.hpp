#pragma once

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/process/pipe.hpp>

#include <iostream>
#include <queue>

namespace nutc {
namespace wrappers {

namespace ba = boost::asio;
namespace bp = boost::process;

/**
 * @brief Singleton that handles async receiving of messages from incoming pipes
 * @note Currently, incoming pipes are exclusively from Wrappers
 */
class AsyncPipeReceiver {
    ba::io_context ios{};
    std::thread ios_thread;
    ba::executor_work_guard<ba::io_context::executor_type> work_guard;

    std::mutex message_lock_{};
    std::queue<std::string> messages_{};
    std::vector<std::shared_ptr<bp::async_pipe>> pipes_{};

    AsyncPipeReceiver() : work_guard(ios.get_executor()) {}

    void async_read_pipe(std::shared_ptr<bp::async_pipe> pipe);

public:
    ~AsyncPipeReceiver();
    void close();

    AsyncPipeReceiver& operator=(const AsyncPipeReceiver&) = delete;
    AsyncPipeReceiver& operator=(AsyncPipeReceiver&&) = delete;
    AsyncPipeReceiver(AsyncPipeReceiver&&) = delete;
    AsyncPipeReceiver(const AsyncPipeReceiver&) = delete;

    static AsyncPipeReceiver&
    get()
    {
        static AsyncPipeReceiver instance{};
        return instance;
    }

    std::shared_ptr<bp::async_pipe> create_pipe(bool start_read);

    std::optional<std::string> get_message();
};

} // namespace wrappers
} // namespace nutc
