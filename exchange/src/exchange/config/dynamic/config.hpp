#pragma once

#include "common/types/decimal.hpp"
#include "common/util.hpp"
#include "exchange/config/static/config.hpp"
#include "ticker_config.hpp"

#include <fmt/format.h>

#include <string>

#include <yaml-cpp/yaml.h>

namespace nutc::exchange {

struct global_config {
    const common::decimal_price STARTING_CAPITAL;
    const size_t WAIT_SECS;
    const unsigned int SANDBOX_TRIAL_SECS;
    const double ORDER_FEE;
    const common::decimal_quantity MAX_CUMULATIVE_OPEN_ORDER_VOLUME;
};

class Config {
    // Globals
    const global_config GLOBAL_CONFIG;
    const std::vector<ticker_config> TICKERS_CONFIG;

public:
    static const Config&
    get()
    {
        static const char* config_file_path = std::getenv("NUTC_CONFIG_FILE");
        static const std::string effective_path =
            config_file_path != nullptr ? config_file_path : DEFAULT_CONFIG_FILE;

        static Config instance(common::find_project_file(effective_path));
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

    Config(const Config&) = delete;
    Config(Config&&) = delete;
    Config operator=(const Config&) = delete;
    Config operator=(Config&&) = delete;
    ~Config() = default;

private:
    explicit Config(const std::string& filename) : Config(YAML::LoadFile(filename)) {}

    explicit Config(const YAML::Node& config) :
        GLOBAL_CONFIG(get_global_config_(config)),
        TICKERS_CONFIG(get_ticker_config_(config))
    {}

    static global_config get_global_config_(const YAML::Node& full_config);
    static std::vector<ticker_config> get_ticker_config_(const YAML::Node& full_config);
    static std::vector<bot_config> get_bot_config_(const YAML::Node& bots_config);
};
} // namespace nutc::exchange
