#include "async_pipe_runner.hpp"

#include <boost/asio/error.hpp>
#include <boost/asio/read_until.hpp>
#include <fmt/format.h>

namespace nutc::exchange {

AsyncPipeRunner::~AsyncPipeRunner()
{
    // ios.reset();
    if (!ios->stopped())
        ios->stop();

    if (ios_thread.joinable())
        ios_thread.join();
}

AsyncPipeRunner::AsyncPipeRunner() :
    ios(std::make_shared<boost::asio::io_context>()), work_guard(ios->get_executor()),
    ios_thread([this]() { ios->run(); })
{}

std::shared_ptr<ba::io_context>
AsyncPipeRunner::get_context()
{
    static AsyncPipeRunner instance{};
    return instance.ios;
}

} // namespace nutc::exchange
