#include "wrapper_handle.hpp"

#include "common/file_operations/file_operations.hpp"
#include "common/firebase/firebase.hpp"
#include "common/messages_exchange_to_wrapper.hpp"
#include "common/util.hpp"

#include <boost/asio.hpp>
#include <fmt/format.h>

namespace {
std::string
quote_id(std::string user_id)
{
    std::replace(user_id.begin(), user_id.end(), '-', ' ');
    return user_id;
}

std::string
force_unwrap_optional(std::optional<std::string> opt, std::string error_msg)
{
    if (!opt.has_value()) [[unlikely]] {
        throw std::runtime_error(error_msg);
    }

    return opt.value();
}

} // namespace

namespace nutc::exchange {

const fs::path&
WrapperHandle::wrapper_binary_path()
{
    static const char* const wrapper_binary_location =
        std::getenv("NUTC_WRAPPER_BINARY_PATH");

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
        kill(wrapper_.id(), SIGTERM);
        wrapper_.wait();
    }
}

namespace {
std::vector<std::string>
create_arguments(
    const std::string& user_id, const std::string& algo_id, bool dev_mode,
    common::AlgoType algo_type
)
{
    std::vector<std::string> args = {
        "--uid", quote_id(user_id), "--algo_id", quote_id(algo_id)
    };

    if (dev_mode) {
        args.emplace_back("--dev");
    }

    if (algo_type == common::AlgoType::binary) {
        args.emplace_back("--binary_algo");
    }
    return args;
}
} // namespace

WrapperHandle::WrapperHandle(
    const std::string& remote_uid, const std::string& algo_id
) :
    WrapperHandle(
        create_arguments(
            remote_uid, algo_id, /*dev_mode=*/false, common::AlgoType::python
        ),
        force_unwrap_optional(
            nutc::common::get_algo(remote_uid, algo_id),
            fmt::format(
                "Could not read algoid {} of uid {} from firebase", algo_id, remote_uid
            )
        )
    )
{}

WrapperHandle::WrapperHandle(const std::string& algo_path, common::AlgoType algo_type) :
    WrapperHandle(
        create_arguments(algo_path, algo_path, /*dev_mode=*/true, algo_type),
        force_unwrap_optional(
            nutc::common::read_file_content(algo_path),
            fmt::format("Could not read algorithm file at {}", algo_path)
        )
    )
{}

void
WrapperHandle::block_on_init()
{
    while (!std::holds_alternative<common::init_message>(reader_->get_message())) {}
}

WrapperHandle::WrapperHandle(
    const std::vector<std::string>& args, const std::string& algorithm
) : reader_(std::make_unique<PipeReader>()), writer_(std::make_unique<PipeWriter>())
{
    static const std::string path{wrapper_binary_path()};

    auto& pipe_in_ptr = reader_->get_pipe();
    auto& pipe_out_ptr = writer_->get_pipe();

    wrapper_ = bp::child(
        bp::exe(path), bp::args(args), bp::std_in<pipe_out_ptr, bp::std_err> stderr,
        bp::std_out > pipe_in_ptr
    );

    using algorithm_t = nutc::common::algorithm_content;
    algorithm_t algorithm_message = algorithm_t(common::base64_encode(algorithm));

    auto encoded_message = glz::write_json(algorithm_message);
    if (!encoded_message.has_value()) [[unlikely]] {
        throw std::runtime_error(glz::format_error(encoded_message.error()));
    }

    writer_->send_message(*encoded_message);
    block_on_init();
}

} // namespace nutc::exchange
