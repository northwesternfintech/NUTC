#include "async_pipe_runner.hpp"

#include <boost/asio/error.hpp>
#include <boost/asio/read_until.hpp>
#include <fmt/format.h>

namespace nutc {
namespace wrappers {

void
AsyncPipeRunner::remove_pipe(std::shared_ptr<bp::async_pipe> pipe)
{
    std::lock_guard<std::mutex> lock{pipe_lock_};
    pipe->cancel();

    std::erase_if(pipes_, [&pipe](const auto& pipe_ptr) { return pipe_ptr == pipe; });
}

AsyncPipeRunner::~AsyncPipeRunner()
{
    for (const std::shared_ptr<bp::async_pipe>& pipe : pipes_) {
        pipe->cancel();
    }

    if (!ios.stopped())
        ios.stop();

    if (ios_thread.joinable())
        ios_thread.join();
}

std::weak_ptr<bp::async_pipe>
AsyncPipeRunner::create_pipe()
{
    std::lock_guard<std::mutex> lock{pipe_lock_};
    pipes_.emplace_back(std::make_shared<bp::async_pipe>(ios));
    return pipes_.back();
}

} // namespace wrappers
} // namespace nutc
