#pragma once

#include "lint/lint_result.hpp"

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/process.hpp>

#include <filesystem>

namespace nutc {
namespace spawning {

enum class AlgoLanguage {
    Python,
    Cpp
};

namespace ba = boost::asio;

class LintProcessManager {
public:
    const std::filesystem::path& spawner_binary_path();
    nutc::lint::lint_result spawn_client(const std::string&, AlgoLanguage language);

private:
    ba::io_context io_context{};
};

} // namespace spawning
} // namespace nutc
