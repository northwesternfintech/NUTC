#include "lint_result.hpp"

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/process.hpp>

namespace ba = boost::asio;

namespace nutc {
namespace spawning {
class SpawnerManager {
public:
    const std::filesystem::path& spawner_binary_path();
    nutc::lint::lint_result spawn_client(const std::string&);

private:
    ba::io_context io_context{};
};

} // namespace spawning
} // namespace nutc
