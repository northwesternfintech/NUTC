#include "mock_api.hpp"

namespace nutc {
namespace mock_api {
std::function<bool(const std::string&, int, bool, const std::string&)>
getMarketFunc()
{
    return
        [](const std::string& security, int quantity, bool side, const std::string& type
        ) {
            log_i(
                mock_api,
                "Mock API: Placing order for {} {} {} {}",
                security,
                quantity,
                side,
                type
            );
            return true;
        };
}
} // namespace mock_api
} // namespace nutc
