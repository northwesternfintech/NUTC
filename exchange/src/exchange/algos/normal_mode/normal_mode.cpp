#include "normal_mode.hpp"

#include "exchange/config.h"
#include "exchange/curl/curl.hpp"
#include "exchange/traders/trader_types/algo_trader.hpp"
#include "shared/config/config_loader.hpp"

#include <fmt/format.h>

#include <algorithm>

namespace nutc {
namespace algos {

namespace {
std::optional<std::string>
get_algo(const std::string& type, const auto& algos)
{
    std::string last{};

    for (auto algo_pair = algos.rbegin(); algo_pair != algos.rend(); ++algo_pair) {
        glz::json_t algo = algo_pair->second;
        if (!algo.contains("downloadURL"))
            continue;

        if (!algo.contains("lintResults"))
            continue;

        // No case type seen
        if (!algo.contains("caseType")) {
            return algo_pair->first;
        }

        if (last.empty() && algo["caseType"].get<std::string>() != type)
            last = algo_pair->first;

        return algo_pair->first;
    }
    if (last.empty())
        return std::nullopt;
    return last;
}
} // namespace

void
NormalModeAlgoInitializer::initialize_trader_container(traders::TraderContainer& traders
) const
{
    static std::string mode = std::getenv("NUTC_CASE");
    if (!(mode == "HFT" || mode == "CRYPTO"))
        throw std::runtime_error("Invalid mode");
    constexpr const std::array<const char*, 3> REQUIRED_DB_FIELDS = {
        "algos", "firstName", "lastName"
    };
    const int starting_capital = config::Config::get().constants().STARTING_CAPITAL;

    static std::random_device rd;
    std::mt19937 g(rd());
    glz::json_t::object_t firebase_users = get_remote_traders();
    std::vector<std::string> ids{};
    for (const auto& user_it : firebase_users) {
        ids.push_back(user_it.first);
    }
    std::shuffle(ids.begin(), ids.end(), g);

    for (const auto& user_id : ids) {
        const auto& user = firebase_users[user_id];

        if (user.contains("isPairedTo"))
            continue;

        bool contains_all_fields =
            std::ranges::all_of(REQUIRED_DB_FIELDS, [&user](const char* field) {
                return user.contains(field);
            });
        if (!contains_all_fields) {
            continue;
        }

        std::string full_name = fmt::format(
            "{} {}", user["firstName"].get<std::string>(),
            user["lastName"].get<std::string>()
        );
        auto algo_id = get_algo(mode, user["algos"].get<glz::json_t::object_t>());
        if (!algo_id.has_value())
            continue;
        traders.add_trader<traders::LocalTrader>(
            user_id, algo_id.value(), full_name, starting_capital
        );
    }
}

glz::json_t::object_t
NormalModeAlgoInitializer::get_remote_traders()
{
    const std::string endpoint = util::get_firebase_endpoint("users.json");
    glz::json_t res = curl::request_to_json("GET", endpoint);
    return res.get<glz::json_t::object_t>();
}

} // namespace algos
} // namespace nutc
