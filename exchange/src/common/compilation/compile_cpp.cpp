#include "compile_cpp.hpp"

#include "common/util.hpp"

#include <optional>

namespace nutc::common {
namespace {

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
        "g++ -std=c++20 -fPIC -O3 -shared -o {} -include {} {}", binary_output,
        filepath.string(), find_project_file("template.cpp")
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
