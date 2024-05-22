#include "config.hpp"

#include <yaml-cpp/yaml.h>

namespace nutc {
namespace config {
namespace {
void
throw_undef_err(const std::string& undefined_err)
{
    throw std::runtime_error(
        fmt::format("{} is not defined in configuration file", undefined_err)
    );
}

} // namespace

std::vector<bot_config>
Config::get_bot_config_(const YAML::Node& bots_config)
{
    std::vector<bot_config> bots;
    for (const auto& bot_n : bots_config) {
        const auto& type_n = bot_n["type"];
        const auto& num_bots_n = bot_n["number_of_bots"];
        const auto& avg_cap_n = bot_n["average_capital"];
        const auto& std_dev_cap_n = bot_n["std_dev_capital"];

        if (!type_n.IsDefined())
            throw_undef_err("bots/{bot}/type");
        if (!num_bots_n.IsDefined())
            throw_undef_err("bots/{bot}/number_of_bots");
        if (!avg_cap_n.IsDefined())
            throw_undef_err("bots/{bot}/average_capital");
        if (!std_dev_cap_n.IsDefined())
            throw_undef_err("bots/{bot}/std_dev_capital");

        auto type_s = type_n.as<std::string>();
        BotType type{};
        if (type_s == "market_maker")
            type = BotType::market_maker;
        else if (type_s == "retail")
            type = BotType::retail;
        else
            throw std::runtime_error("Unknown bot type");

        auto num_bots = num_bots_n.as<size_t>();
        auto avg_cap = avg_cap_n.as<double>();
        auto std_dev_cap = std_dev_cap_n.as<double>();
        bots.emplace_back(bot_config{type, num_bots, avg_cap, std_dev_cap});
    }
    return bots;
}

std::vector<ticker_config>
Config::get_ticker_config_(const YAML::Node& full_config)
{
    const auto& tickers_n = full_config["tickers"];
    if (!tickers_n.IsDefined() || !tickers_n.IsSequence())
        throw_undef_err("tickers");

    std::vector<ticker_config> tickers;
    for (const auto& ticker_n : tickers_n) {
        const auto& ticker_symb = ticker_n["ticker"];
        const auto& ticker_start_price = ticker_n["start_price"];
        const auto& bots = ticker_n["bots"];

        if (!ticker_symb.IsDefined())
            throw_undef_err("tickers/{ticker}/ticker");

        const std::string& ticker_symb_s = ticker_symb.as<std::string>();
        if (!ticker_start_price.IsDefined())
            throw_undef_err(fmt::format("tickers/{}/ticker", ticker_symb_s));

        std::vector<bot_config> bot_config{};
        if (bots.IsDefined() && bots.IsSequence())
            bot_config = get_bot_config_(bots);

        tickers.emplace_back(
            ticker_config{ticker_symb_s, ticker_start_price.as<double>(), bot_config}
        );
    }
    return tickers;
}

global_config
Config::get_global_config_(const YAML::Node& full_config)
{
    const auto& global = full_config["global"];
    if (!global.IsDefined())
        throw_undef_err("global");
    const auto& starting_capital = global["starting_capital"];
    const auto& wait_secs = global["wait_secs"];
    const auto& exp_ticks = global["order_expiration_ticks"];
    const auto& tick_hz = global["exchange_tick_hz"];
    const auto& sandbox_secs = global["sandbox_trial_seconds"];
    const auto& order_fee = global["order_fee"];
    if (!starting_capital.IsDefined())
        throw_undef_err("global/starting_capital");
    if (!wait_secs.IsDefined())
        throw_undef_err("global/wait_secs");
    if (!exp_ticks.IsDefined())
        throw_undef_err("global/order_expiration_ticks");
    if (!tick_hz.IsDefined())
        throw_undef_err("global/exchange_tick_hz");
    if (!sandbox_secs.IsDefined())
        throw_undef_err("global/sandbox_trial_seconds");
    return {starting_capital.as<int>(),
            wait_secs.as<size_t>(),
            exp_ticks.as<size_t>(),
            tick_hz.as<uint16_t>(),
            sandbox_secs.as<unsigned int>(),
            order_fee.IsDefined() ? order_fee.as<double>() : 0};
}

} // namespace config
} // namespace nutc
