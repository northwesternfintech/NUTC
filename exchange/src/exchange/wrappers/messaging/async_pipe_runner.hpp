#pragma once

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/process/pipe.hpp>

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
    ba::executor_work_guard<ba::io_context::executor_type> work_guard;
    std::thread ios_thread;

    std::mutex pipe_lock_{};
    std::vector<std::shared_ptr<bp::async_pipe>> pipes_{};

    AsyncPipeRunner() :
        work_guard(ios.get_executor()), ios_thread([this]() { ios.run(); })
    {}

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

    std::weak_ptr<bp::async_pipe> create_pipe();
    void remove_pipe(std::shared_ptr<bp::async_pipe> pipe);
};

} // namespace wrappers
} // namespace nutc
