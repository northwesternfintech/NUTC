#pragma once

#include "common/util.hpp"
#include "linter/lint/lint_result.hpp"

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/process.hpp>

namespace nutc::linter {

lint_result spawn_client(const std::string&, common::AlgoLanguage language);

} // namespace nutc::linter
