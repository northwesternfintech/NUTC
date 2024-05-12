#pragma once

#include "shared/config/config.h"

#include <fmt/format.h>

#include <string>

#include <yaml-cpp/yaml.h>

namespace nutc {
namespace config {

enum class BotType { market_maker, retail, bollinger };

struct global_config {
    const int STARTING_CAPITAL;
    const size_t WAIT_SECS;
    const size_t ORDER_EXPIRATION_TICKS;
    const uint16_t TICK_HZ;
    const uint8_t DISPLAY_HZ;
    const unsigned int SANDBOX_TRIAL_SECS;
    const double ORDER_FEE;
};

struct ticker_config {
    const std::string TICKER;
    const double STARTING_PRICE;
};

struct bot_config {
    const std::string ASSOC_TICKER;
    const BotType TYPE;
    const size_t NUM_BOTS;
    const double AVERAGE_CAPITAL;
    const double STD_DEV_CAPITAL;
};

class Config {
    // Globals
    const global_config GLOBAL_CONFIG;
    const std::vector<ticker_config> TICKERS_CONFIG;
    const std::vector<bot_config> BOTS_CONFIG;

public:
    static const Config&
    get()
    {
        static const char* config_file_path = std::getenv("NUTC_CONFIG_FILE");
        static const char* effective_path =
            config_file_path ? config_file_path : DEFAULT_CONFIG_FILE;

        static Config instance(effective_path);
        return instance;
    }

    const global_config&
    constants() const
    {
        return GLOBAL_CONFIG;
    }

    const std::vector<ticker_config>&
    get_tickers() const
    {
        return TICKERS_CONFIG;
    }

    const std::vector<bot_config>&
    get_bots() const
    {
        return BOTS_CONFIG;
    }

    Config(const Config&) = delete;
    Config(Config&&) = delete;
    Config operator=(const Config&) = delete;
    Config operator=(Config&&) = delete;
    ~Config() = default;

private:
    explicit Config(const std::string& filename) : Config(YAML::LoadFile(filename)) {}

    explicit Config(const YAML::Node& config) :
        GLOBAL_CONFIG(get_global_config_(config)),
        TICKERS_CONFIG(get_ticker_config_(config)), BOTS_CONFIG(get_bot_config_(config))
    {}

    static global_config get_global_config_(const YAML::Node& full_config);
    static std::vector<ticker_config> get_ticker_config_(const YAML::Node& full_config);
    static std::vector<bot_config> get_bot_config_(const YAML::Node& full_config);
};
} // namespace config
} // namespace nutc
