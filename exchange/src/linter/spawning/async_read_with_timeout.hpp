#pragma once

#include <boost/asio.hpp>
#include <boost/process.hpp>

namespace nutc::linter {
namespace ba = boost::asio;
namespace bp = boost::process;

class AsyncReadWithTimeout {
public:
    AsyncReadWithTimeout(
        ba::io_context& io_context, bp::async_pipe& pipe, ba::streambuf& buffer,
        std::chrono::seconds timeout,
        std::function<void(const boost::system::error_code&, std::size_t)> handler
    );

private:
    void handle_read_(
        const boost::system::error_code& err_code, std::size_t bytes_transferred
    );

    void handle_timeout_(const boost::system::error_code& err_code);

    bp::async_pipe& pipe_;
    boost::asio::streambuf& buffer_;
    boost::asio::steady_timer timeout_timer_;
    std::function<void(const boost::system::error_code&, std::size_t)> read_handler_;
};
} // namespace nutc::linter
