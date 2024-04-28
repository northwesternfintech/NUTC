#pragma once

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/process/pipe.hpp>

#include <iostream>
#include <optional>
#include <queue>

namespace nutc {
namespace wrappers {

namespace ba = boost::asio;
namespace bp = boost::process;

/**
 * @brief Singleton that handles async io messages from  pipes
 * @note Mostly thread safe
 * @note This allows us to have one thread for all pipes
 */
class AsyncPipeRunner {
    ba::io_context ios{};
    std::thread ios_thread;
    ba::executor_work_guard<ba::io_context::executor_type> work_guard;

    std::mutex message_lock_{};
    std::queue<std::string> messages_{};
    std::vector<std::shared_ptr<bp::async_pipe>> pipes_{};

    AsyncPipeRunner() : work_guard(ios.get_executor()) {}

    void
    async_read_pipe(std::shared_ptr<bp::async_pipe> pipe)
    {
        return async_read_pipe(std::move(pipe), std::make_shared<std::string>());
    }

    void async_read_pipe(
        std::shared_ptr<bp::async_pipe> pipe, std::shared_ptr<std::string> buf
    );

public:
    ~AsyncPipeRunner();
    void close();

    AsyncPipeRunner& operator=(const AsyncPipeRunner&) = delete;
    AsyncPipeRunner& operator=(AsyncPipeRunner&&) = delete;
    AsyncPipeRunner(AsyncPipeRunner&&) = delete;
    AsyncPipeRunner(const AsyncPipeRunner&) = delete;

    static AsyncPipeRunner&
    get()
    {
        static AsyncPipeRunner instance{};
        return instance;
    }

    std::weak_ptr<bp::async_pipe> create_pipe(bool start_read);
    void remove_pipe(std::shared_ptr<bp::async_pipe> pipe);

    std::optional<std::string> get_message();
};

} // namespace wrappers
} // namespace nutc
