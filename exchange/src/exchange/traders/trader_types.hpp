#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

namespace nutc {
namespace manager {

enum TraderType { REMOTE, LOCAL, BOT };

struct generic_trader_t {
    std::string
    get_id() const
    {
        return USER_ID;
    }

    virtual TraderType
    get_type() const
    {
        throw std::runtime_error("Not implemented");
    }

    virtual ~generic_trader_t() = default;

    float
    get_capital() const
    {
        return capital_;
    }

    bool
    is_active() const
    {
        return is_active_;
    }

    void
    set_active(bool active)
    {
        is_active_ = active;
    }

    bool
    has_start_delay() const
    {
        return has_start_delay_;
    }

    void
    set_start_delay(bool start_delay)
    {
        has_start_delay_ = start_delay;
    }

    float
    get_holdings(const std::string& ticker) const
    {
        if (!holdings_.contains(ticker))
            return 0.0f;

        return holdings_.at(ticker);
    }

    float
    modify_holdings(const std::string& ticker, float change_in_holdings)
    {
        holdings_[ticker] += change_in_holdings;
        return holdings_[ticker];
    }

    void
    set_capital(float capital)
    {
        capital_ = capital;
    }

    void
    modify_capital(float change_in_capital)
    {
        capital_ += change_in_capital;
    }

    explicit generic_trader_t(std::string user_id) : USER_ID(std::move(user_id)) {}

private:
    const std::string USER_ID;
    float capital_ = 0;
    bool is_active_ = false;
    bool has_start_delay_ = true;
    std::unordered_map<std::string, float> holdings_{};
};

struct remote_trader_t : public generic_trader_t {
    remote_trader_t(std::string user_id, std::string algo_id) :
        generic_trader_t(std::move(user_id)), algo_id_(std::move(algo_id))
    {}

    TraderType
    get_type() const override
    {
        return TraderType::REMOTE;
    }

    void
    set_pid(pid_t pid)
    {
        pid_ = pid;
    }

    pid_t
    get_pid() const
    {
        return pid_;
    }

    std::string
    get_algo_id() const
    {
        return algo_id_;
    }

private:
    std::string algo_id_;
    pid_t pid_{};
};

struct local_trader_t : public generic_trader_t {
    explicit local_trader_t(std::string algo_path) :
        generic_trader_t(generate_user_id()), ALGO_PATH(std::move(algo_path))
    {}

    TraderType
    get_type() const override
    {
        return TraderType::LOCAL;
    }

    // FOR TESTING PURPOSES ONLY
    explicit local_trader_t(std::string user_id, std::string algo_path) :
        generic_trader_t(std::move(user_id)), ALGO_PATH(std::move(algo_path))
    {}

    std::string
    get_algo_path() const
    {
        return ALGO_PATH;
    }

    void
    set_pid(pid_t pid)
    {
        pid_ = pid;
    }

    pid_t
    get_pid() const
    {
        return pid_;
    }

private:
    const std::string ALGO_PATH;
    pid_t pid_{};

    static uint
    get_and_increment_user_id()
    {
        static uint user_id = 0;
        return user_id++;
    }

    static std::string
    generate_user_id()
    {
        return "LOCAL_" + std::to_string(get_and_increment_user_id());
    }
};

struct bot_trader_t : public generic_trader_t {
    bot_trader_t() : generic_trader_t(generate_user_id()) {}

private:
    TraderType
    get_type() const override
    {
        return TraderType::BOT;
    }

    static uint
    get_and_increment_user_id()
    {
        static uint user_id = 0;
        return user_id++;
    }

    static std::string
    generate_user_id()
    {
        return "BOT_" + std::to_string(get_and_increment_user_id());
    }
};

using trader_t = std::variant<remote_trader_t, local_trader_t, bot_trader_t>;

} // namespace manager
} // namespace nutc
