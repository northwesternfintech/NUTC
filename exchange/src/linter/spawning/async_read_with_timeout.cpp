#include "async_read_with_timeout.hpp"

namespace nutc::linter {
AsyncReadWithTimeout::AsyncReadWithTimeout(
    ba::io_context& io_context, bp::async_pipe& pipe, ba::streambuf& buffer,
    std::chrono::milliseconds timeout,
    std::function<void(const boost::system::error_code&, std::size_t)> handler
) :
    pipe_(pipe), buffer_(buffer), timeout_timer_(io_context),
    read_handler_(std::move(handler))
{
    ba::async_read_until(
        pipe_, buffer_, '\n',
        [this](
            const boost::system::error_code& err_code, std::size_t bytes_transferred
        ) { this->handle_read_(err_code, bytes_transferred); }
    );

    timeout_timer_.expires_after(timeout);
    timeout_timer_.async_wait([this](const boost::system::error_code& err_code) {
        this->handle_timeout_(err_code);
    });
}

void
AsyncReadWithTimeout::handle_read_(
    const boost::system::error_code& err_code, std::size_t bytes_transferred
)
{
    timeout_timer_.cancel();
    read_handler_(err_code, bytes_transferred);
}

void
AsyncReadWithTimeout::handle_timeout_(const boost::system::error_code& err_code)
{
    if (!err_code) {
        pipe_.cancel();
        read_handler_(boost::asio::error::operation_aborted, 0);
    }
}
} // namespace nutc::linter
