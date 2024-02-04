#pragma once

#include "bot_container.hpp"

namespace nutc {
namespace bots {

// TODO(stevenewald): in the future, force manual creation of bots (vs automatic if
// ticker not found)
class BotContainerMapper {
    std::unordered_map<std::string, BotContainer> instances_;
    BotContainerMapper() = default;

    BotContainer&
    get_container_(const std::string& ticker, float starting_price)
    {
        auto it = instances_.find(ticker);
        if (it == instances_.end()) {
            instances_[ticker] = BotContainer(ticker, starting_price);
        }
        return instances_[ticker];
    }

    ~BotContainerMapper() = default;

public:
    static BotContainer&
    get_instance(const std::string& ticker = "DEFAULT", float starting_price = 1000.0f)
    {
        static BotContainerMapper instances;
        return instances.get_container_(ticker, starting_price);
    }

    BotContainerMapper(BotContainerMapper const&) = delete;
    BotContainerMapper(BotContainerMapper&&) = delete;
    BotContainerMapper& operator=(BotContainerMapper&&) = delete;
    BotContainerMapper& operator=(BotContainerMapper const&) = delete;
};
} // namespace bots
} // namespace nutc
