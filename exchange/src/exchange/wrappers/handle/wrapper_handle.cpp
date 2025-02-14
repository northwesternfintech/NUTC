#include "wrapper_handle.hpp"

#include "common/types/algorithm/algorithm.hpp"
#include "common/types/messages/messages_exchange_to_wrapper.hpp"
#include "common/util.hpp"

#include <boost/asio.hpp>
#include <fmt/format.h>
#include <glaze/json/write.hpp>

namespace {
std::string
quote_id(std::string user_id)
{
    std::replace(user_id.begin(), user_id.end(), '-', ' ');
    return user_id;
}

} // namespace

namespace nutc::exchange {

WrapperHandle::~WrapperHandle()
{
    if (wrapper_.running()) {
        kill(wrapper_.id(), SIGTERM);
        wrapper_.wait();
    }
}

WrapperHandle::WrapperHandle(const common::algorithm_variant& algo_variant) :
    WrapperHandle(
        create_arguments(algo_variant),
        std::visit([](auto& algo) { return algo.get_algo_string(); }, algo_variant)
    )
{}

void
WrapperHandle::block_on_init()
{
    while (!std::holds_alternative<common::init_message>(reader_.get_message())) {}
}

WrapperHandle::WrapperHandle(
    const std::vector<std::string>& args, const std::string& algo_string
)
{
    auto& pipe_in_ptr = reader_.get_pipe();
    auto& pipe_out_ptr = writer_.get_pipe();

    wrapper_ = bp::child(
        bp::exe(common::find_project_file("WRAPPER")), bp::args(args),
        bp::std_in<pipe_out_ptr, bp::std_err> stderr, bp::std_out > pipe_in_ptr
    );

    using algorithm_t = nutc::common::algorithm_content;
    algorithm_t algorithm_message = algorithm_t(common::base64_encode(algo_string));

    auto encoded_message = glz::write_json(algorithm_message);
    if (!encoded_message.has_value()) [[unlikely]] {
        throw std::runtime_error(glz::format_error(encoded_message.error()));
    }

    writer_.send_message(*encoded_message);
    block_on_init();
}

std::vector<std::string>
WrapperHandle::create_arguments(const common::algorithm_variant& algo_variant)
{
    std::vector<std::string> args = {"--uid", quote_id(common::get_id(algo_variant))};

    auto language =
        std::visit([](auto& algo) { return algo.get_language(); }, algo_variant);

    if (language == common::AlgoLanguage::cpp) {
        args.emplace_back("--cpp_algo");
    }
    else if (language == common::AlgoLanguage::python) {
        args.emplace_back("--python_algo");
    }
    return args;
}

} // namespace nutc::exchange
