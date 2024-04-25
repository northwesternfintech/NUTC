#include "wrapper_handle.hpp"

#include "exchange/config.h"
#include "exchange/wrappers/messaging/async_pipe_runner.hpp"
#include "shared/config/config_loader.hpp"

#include <boost/asio.hpp>
#include <fmt/format.h>

namespace {
std::string
quote_id(std::string user_id)
{
    std::replace(user_id.begin(), user_id.end(), '-', ' ');
    return user_id;
}

} // namespace

namespace nutc {
namespace wrappers {

const fs::path&
WrapperHandle::wrapper_binary_path()
{
    static const char* wrapper_binary_location =
        std::getenv("NUTC_WRAPPER_BINARY_PATH"); // NOLINT
    if (wrapper_binary_location == nullptr) [[unlikely]] {
        throw std::runtime_error("NUTC_WRAPPER_BINARY_PATH environment variable not set"
        );
    }

    static const fs::path wrapper_binary_path{wrapper_binary_location};
    if (!fs::exists(wrapper_binary_path))
        throw std::runtime_error("File at NUTC_WRAPPER_BINARY_PATH does not exist");

    return wrapper_binary_path;
}

WrapperHandle::~WrapperHandle()
{
    if (wrapper_.running()) {
        wrapper_.terminate();
        wrapper_.wait();
    }
}

WrapperHandle::WrapperHandle(const std::string& remote_uid, const std::string& algo_id)
{
    std::vector<std::string> args{
        "--uid", quote_id(remote_uid), "--algo_id", quote_id(algo_id)
    };
    spawn_wrapper(args);
}

WrapperHandle::WrapperHandle(const std::string& algo_path)
{
    std::vector<std::string> args{
        "--uid", quote_id(algo_path), "--algo_id", quote_id(algo_path), "--dev"
    };
    spawn_wrapper(args);
}

void
WrapperHandle::spawn_wrapper(const std::vector<std::string>& args)
{
    static const std::string path{wrapper_binary_path()};
    // While we receive a shared_ptr to the pipe, we dereference it to get the actual
    // pipe. This is risky, as we need to make sure we close the pipe before we shut
    // down the wrapper
    pipe_in = AsyncPipeRunner::get().create_pipe(true);
    pipe_out = AsyncPipeRunner::get().create_pipe(false);
    auto pipe_in_ptr = pipe_in.lock();
    auto pipe_out_ptr = pipe_out.lock();
    assert(pipe_in_ptr != nullptr);
    assert(pipe_out_ptr != nullptr);
    wrapper_ = bp::child(
        bp::exe(path), bp::args(args), bp::std_in<*pipe_out_ptr, bp::std_err> bp::null,
        bp::std_out > *pipe_in_ptr
    );
}

void
WrapperHandle::send_messages(std::vector<std::string> messages)
{
    if (!wrapper_.running()) [[unlikely]]
        return;
    std::lock_guard<std::mutex> lock{messages_lock_};
    if (queued_messages_.size() > MAX_OUTGOING_MQ_SIZE) [[unlikely]] {
        return;
    }
    std::ranges::move(messages, std::back_inserter(queued_messages_));

    // It will enqueue these messages anyway
    if (is_writing_.test_and_set(std::memory_order_acquire)) {
        return;
    }
    auto to_send = queued_messages_.front();
    queued_messages_.pop_front();
    async_write_pipe(to_send + "\n");
}

void
WrapperHandle::async_write_pipe(std::string message)
{
    auto buf = std::make_shared<std::string>(std::move(message));
    auto pipe_out_ptr = pipe_out.lock();
    if (pipe_out_ptr == nullptr)
        return;
    boost::asio::async_write(
        *pipe_out_ptr, bp::buffer(*buf),
        [this, buf](boost::system::error_code ec, std::size_t) {
            if (!ec) [[likely]] {
                std::lock_guard<std::mutex> lock{messages_lock_};
                if (queued_messages_.empty()) {
                    is_writing_.clear(std::memory_order_release);
                    return;
                }

                std::string to_write = queued_messages_.front();
                queued_messages_.pop_front();
                async_write_pipe(to_write + "\n");
            }
            else {
                // Ignore the error, but stop writing
                is_writing_.clear(std::memory_order_release);
            }
        }
    );
}

} // namespace wrappers
} // namespace nutc
