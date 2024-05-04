#include "mock_api.hpp"

namespace nutc {
namespace mock_api {

std::function<bool(const std::string&, const std::string&, float, float)>
getMarketFunc()
{
    return [](const std::string&, const std::string&, float, float) { return true; };
}
} // namespace mock_api
} // namespace nutc
