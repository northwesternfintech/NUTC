#include "wrapper_handle.hpp"

#include "exchange/wrappers/messaging/async_pipe_runner.hpp"
#include "shared/util.hpp"

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
    auto pipe_in_ptr = reader_.get_pipe().lock();
    auto pipe_out_ptr = writer_.get_pipe().lock();
    assert(pipe_in_ptr != nullptr);
    assert(pipe_out_ptr != nullptr);
    wrapper_ = bp::child(
        bp::exe(path), bp::args(args), bp::std_in<*pipe_out_ptr, bp::std_err> stderr,
        bp::std_out > *pipe_in_ptr
    );
}

} // namespace wrappers
} // namespace nutc
