#pragma once
#include "exchange/tickers/engine/order_container.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

#include <vector>

namespace nutc {
namespace matching {
using messages::ObUpdate;

class LevelUpdateGenerator {
public:
    static std::vector<ObUpdate>
    get_updates(
        const std::string& ticker, const OrderContainer& before,
        const OrderContainer& after
    );
};

} // namespace matching
} // namespace nutc
