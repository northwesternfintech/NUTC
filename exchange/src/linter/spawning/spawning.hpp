#pragma once

#include "common/util.hpp"
#include "linter/config.h"
#include "linter/lint/lint_result.hpp"

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/process.hpp>

namespace nutc::linter {

lint_result spawn_client(
    const std::string&, common::AlgoLanguage language,
    std::chrono::milliseconds timeout =
        std::chrono::milliseconds{LINT_AUTO_TIMEOUT_MILLISECONDS}
);

} // namespace nutc::linter
