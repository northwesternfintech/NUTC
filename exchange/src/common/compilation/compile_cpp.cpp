#include "compile_cpp.hpp"

#include "common/file_operations/file_operations.hpp"

#include <optional>

namespace nutc::common {
namespace {
std::string
get_cpp_template_path()
{
    static const char* template_path_env = std::getenv("NUTC_CPP_TEMPLATE_PATH");
    if (template_path_env == nullptr) {
        throw std::runtime_error("Template.cpp path not set, unable to compile cpp");
    }
    if (!file_exists(template_path_env)) {
        throw std::runtime_error(
            fmt::format("File {} does not exist", template_path_env)
        );
    }

    return template_path_env;
}

std::optional<std::string>
exec_command(const std::string& command)
{
    std::array<char, 128> buffer{};
    std::string result;

    FILE* pipe = popen((command + " 2>&1").c_str(), "r");
    if (pipe == nullptr) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    if (pclose(pipe) != 0) {
        return result;
    }
    return std::nullopt;
}
} // namespace

std::string
compile_cpp(const std::filesystem::path& filepath)
{
    std::string binary_output = (boost::filesystem::temp_directory_path()
                                 / boost::filesystem::unique_path("%%%%-%%%%-%%%%.tmp"))
                                    .string();

    std::string command = fmt::format(
        "g++ -std=c++20 -fPIC -shared -o {} -include {} {}", binary_output,
        filepath.string(), get_cpp_template_path()
    );

    std::optional<std::string> result = exec_command(command);

    if (result) {
        throw std::runtime_error(fmt::format(
            "Compilation of {} failed. Compiler output below:\n {}", filepath.string(),
            result.value()
        ));
    }
    return binary_output;
}
} // namespace nutc::common
