#include "util.hpp"

#include "common/config/config.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <fmt/format.h>
#include <x86intrin.h>

#include <random>

namespace nutc::common {
namespace bi = boost::archive::iterators;

namespace {
std::string
find_path_nocache(const std::string& file_name)
{
    boost::filesystem::path exe_path = boost::dll::program_location();

    std::vector<boost::filesystem::path> possible_paths = {
        exe_path.parent_path() / file_name,
        exe_path.parent_path().parent_path() / file_name,
        exe_path.parent_path().parent_path().parent_path() / file_name,
        exe_path.parent_path().parent_path().parent_path().parent_path() / file_name,
    };

    auto path = std::ranges::find_if(possible_paths, [](auto& path) {
        return boost::filesystem::exists(path);
    });
    if (path != possible_paths.end()) {
        return path->string();
    }

    throw std::runtime_error(fmt::format("{} not found", file_name));
}

} // namespace

std::string
find_project_file(const std::string& file_name)
{
    static std::unordered_map<std::string, std::string> path_cache;
    if (path_cache.contains(file_name)) {
        return path_cache[file_name];
    }
    return path_cache[file_name] = find_path_nocache(file_name);
}

// We only want one thread per process generating orders (bots in exchange, individual
// algos in other processes). That makes this thread safe

order_id_t
generate_order_id()
{
    static order_id_t pid = getpid();
    static order_id_t start_order_count = pid << 46;
    return ++start_order_count;
}

uint64_t
get_time()
{
    return __rdtsc();
}

std::string
to_string(Side side)
{
    return side == Side::buy ? "BUY" : "SELL";
}

// https://stackoverflow.com/questions/7053538/how-do-i-encode-a-string-to-base64-using-only-boost
std::string
base64_encode(const std::string& data)
{
    using base64_it =
        bi::base64_from_binary<bi::transform_width<std::string::const_iterator, 6, 8>>;

    auto tmp = std::string(base64_it(data.begin()), base64_it(data.end()));
    return tmp.append((3 - data.size() % 3) % 3, '=');
}

std::string
base64_decode(const std::string& data)
{
    using base64_it =
        bi::transform_width<bi::binary_from_base64<std::string::const_iterator>, 8, 6>;

    return boost::algorithm::trim_right_copy_if(
        std::string(base64_it(data.begin()), base64_it(data.end())),
        [](char chr) { return chr == '\0'; }
    );
}

std::string
trader_id(const std::string& user_id, const std::string& algo_id)
{
    return fmt::format("{}-{}", user_id, algo_id);
}

} // namespace nutc::common
