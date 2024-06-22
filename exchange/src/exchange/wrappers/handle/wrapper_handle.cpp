#include "wrapper_handle.hpp"

#include "exchange/wrappers/messaging/async_pipe_runner.hpp"
#include "shared/util.hpp"
#include "shared/firebase/firebase.hpp"
#include "shared/file_operations/file_operations.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

#include <boost/asio.hpp>
#include <fmt/format.h>

#include <iostream>

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

WrapperHandle::WrapperHandle(
    const std::string& remote_uid, const std::string& algo_id
) : WrapperHandle(
        {"--uid", quote_id(remote_uid), "--algo_id", quote_id(algo_id)},
        nutc::firebase::get_algo(remote_uid, algo_id)
    )
{}

WrapperHandle::WrapperHandle(const std::string& algo_path) :
    WrapperHandle(
        {"--uid", quote_id(algo_path), "--algo_id", quote_id(algo_path), "--dev"},
        nutc::file_ops::read_file_content(algo_path)
    )
{}

void
WrapperHandle::block_on_init()
{
    auto message = reader_.get_message();
    if (std::holds_alternative<init_message>(message)) {
        return;
    }
    throw std::runtime_error("Received non-init message on initialization");
}

WrapperHandle::WrapperHandle(const std::vector<std::string>& args, const std::optional<std::string>& algorithm)
{
    if (!algorithm.has_value()) {
        throw std::runtime_error("Received empty algorithm; cannot initiate empty wrapper");
    }

    static const std::string path{wrapper_binary_path()};

    auto& pipe_in_ptr = reader_.get_pipe();
    auto& pipe_out_ptr = writer_.get_pipe();

    wrapper_ = bp::child(
        bp::exe(path), bp::args(args), bp::std_in<pipe_out_ptr, bp::std_err> stderr,
        bp::std_out > pipe_in_ptr
    );

    using algorithm_t = nutc::messages::algorithm_content;
    algorithm_t algorithm_message = algorithm_t(algorithm.value());
    auto encoded_message = glz::write_json(algorithm_message);

    // writer_.send_message(encoded_message);
    block_on_init();
}

} // namespace wrappers
} // namespace nutc
