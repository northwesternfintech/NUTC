#include "mock_api.hpp"

namespace nutc {
namespace mock_api {
std::function<
    bool(const std::string&, const std::string&, const std::string&, float, float)>

getMarketFunc()
{
    return [](const std::string& side,
              const std::string& type,
              const std::string& ticker,
              float quantity,
              float price) {
        log_i(
            mock_api,
            "Mock API: Placing order side {} type {} ticker {} quantity {} price "
            "{}",
            side,
            type,
            ticker,
            quantity,
            price
        );
        return true;
    };
}
} // namespace mock_api
} // namespace nutc
