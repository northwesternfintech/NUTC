#include "mock_api.hpp"

namespace nutc {
namespace mock_api {
std::function<bool(const std::string&, float, bool, const std::string&, float)>
getMarketFunc()
{
    return [](const std::string& security,
              float quantity,
              bool side,
              const std::string& type,
              float price) {
        log_i(
            mock_api,
            "Mock API: Placing order for {} {} {} {} {}",
            security,
            quantity,
            side,
            type,
            price
        );
        return true;
    };
}
} // namespace mock_api
} // namespace nutc
