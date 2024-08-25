#pragma once

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/process/pipe.hpp>

namespace nutc::exchange {

namespace ba = boost::asio;

/**
 * @brief Singleton that handles async io shared from  pipes
 * @note Mostly thread safe
 * @note This allows us to have one thread for all pipes
 */
class AsyncPipeRunner {
    std::shared_ptr<ba::io_context> ios;
    ba::executor_work_guard<ba::io_context::executor_type> work_guard;
    std::thread ios_thread;

    AsyncPipeRunner();

public:
    ~AsyncPipeRunner();

    static std::shared_ptr<ba::io_context> get_context();

    AsyncPipeRunner& operator=(const AsyncPipeRunner&) = delete;
    AsyncPipeRunner& operator=(AsyncPipeRunner&&) = delete;
    AsyncPipeRunner(AsyncPipeRunner&&) = delete;
    AsyncPipeRunner(const AsyncPipeRunner&) = delete;
};

} // namespace nutc::exchange
