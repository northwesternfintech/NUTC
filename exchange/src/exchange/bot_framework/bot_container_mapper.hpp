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
    get_container_(const std::string& ticker)
    {
        if (instances_.find(ticker) == instances_.end()) {
            instances_[ticker] = BotContainer(ticker);
        }
        return instances_[ticker];
    }

    ~BotContainerMapper() = default;

public:
    static BotContainer&
    get_instance(const std::string& ticker = "DEFAULT")
    {
        static BotContainerMapper instances;
        return instances.get_container_(ticker);
    }

    BotContainerMapper(BotContainerMapper const&) = delete;
    BotContainerMapper(BotContainerMapper&&) = delete;
    BotContainerMapper& operator=(BotContainerMapper&&) = delete;
    BotContainerMapper& operator=(BotContainerMapper const&) = delete;
};
} // namespace bots
} // namespace nutc
