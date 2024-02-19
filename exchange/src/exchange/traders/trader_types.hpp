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

    virtual TraderType get_type() const = 0;

    double
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

    double
    get_holdings(const std::string& ticker) const
    {
        if (!holdings_.contains(ticker))
            return 0.0;

        return holdings_.at(ticker);
    }

    double
    modify_holdings(const std::string& ticker, double change_in_holdings)
    {
        holdings_[ticker] += change_in_holdings;
        return holdings_[ticker];
    }

    void
    set_capital(double capital)
    {
        capital_ = capital;
    }

    void
    modify_capital(double change_in_capital)
    {
        capital_ += change_in_capital;
    }

    virtual void set_pid(pid_t pid) = 0;
    virtual pid_t get_pid() const = 0;

    virtual std::string get_algo_id() const = 0;

    explicit generic_trader_t(std::string user_id) : USER_ID(std::move(user_id)) {}

    virtual ~generic_trader_t() = default;
    generic_trader_t& operator=(generic_trader_t&& other) = delete;
    generic_trader_t& operator=(const generic_trader_t& other) = delete;
    generic_trader_t(generic_trader_t&& other) = default;
    generic_trader_t(const generic_trader_t& other) = delete;

private:
    const std::string USER_ID;
    double capital_ = 0;
    bool is_active_ = false;
    bool has_start_delay_ = true;
    std::unordered_map<std::string, double> holdings_{};
};

struct remote_trader_t : public generic_trader_t {
    remote_trader_t(std::string user_id, std::string algo_id) :
        generic_trader_t(std::move(user_id)), algo_id_(std::move(algo_id))
    {}

    constexpr TraderType
    get_type() const override
    {
        return TraderType::REMOTE;
    }

    void
    set_pid(pid_t pid) override
    {
        pid_ = pid;
    }

    pid_t
    get_pid() const override
    {
        return pid_;
    }

    std::string
    get_algo_id() const override
    {
        return algo_id_;
    }

private:
    std::string algo_id_;
    pid_t pid_{};
};

struct local_trader_t : public generic_trader_t {
    explicit local_trader_t(std::string algo_path) :
        generic_trader_t(std::move(algo_path))
    {}

    constexpr TraderType
    get_type() const override
    {
        return TraderType::LOCAL;
    }

    std::string
    get_algo_id() const override
    {
        return get_id();
    }

    void
    set_pid(pid_t pid) override
    {
        pid_ = pid;
    }

    pid_t
    get_pid() const override
    {
        return pid_;
    }

private:
    pid_t pid_{};
};

struct bot_trader_t : public generic_trader_t {
    bot_trader_t() : generic_trader_t(generate_user_id()) {}

    constexpr TraderType
    get_type() const override
    {
        return TraderType::BOT;
    }

    pid_t
    get_pid() const override
    {
        return -1;
    }

    void
    set_pid(pid_t) override
    {}

    std::string
    get_algo_id() const override
    {
        throw std::runtime_error("Not implemented");
    }

private:
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
