#pragma once

#include <stdexcept>
#include <string>

#include <rabbitmq-c/amqp.h>

namespace nutc {
namespace rabbitmq {

class RabbitMQConnectionManager {
    // Only allow the singleton instance to create the connection
    RabbitMQConnectionManager& operator=(RabbitMQConnectionManager&&) = default;

    ~RabbitMQConnectionManager() { close_connection(); }

public:
    // Delete the copy constructor and assignment operator
    RabbitMQConnectionManager(const RabbitMQConnectionManager&) = delete;
    RabbitMQConnectionManager& operator=(const RabbitMQConnectionManager&) = delete;

    // Delete the move constructor and assignment operator
    RabbitMQConnectionManager(RabbitMQConnectionManager&&) = delete;

    RabbitMQConnectionManager& operator=(RabbitMQConnectionManager&) = delete;

    // Public method to access the singleton instance
    static RabbitMQConnectionManager&
    get_instance()
    {
        static RabbitMQConnectionManager instance;
        return instance;
    }

    // For testing purposes
    static void
    reset_instance()
    {
        get_instance() = RabbitMQConnectionManager();
    }

    void close_connection();

    [[nodiscard]] bool
    connected_to_rabbitmq() const
    {
        return connected_;
    }

    amqp_connection_state_t&
    get_connection_state()
    {
        if (connection_state_ == nullptr) {
            throw std::runtime_error(
                "Attempted to get connection state while not connected to RMQ"
            );
        }
        return connection_state_;
    }

    bool
    initialize_connection()
    {
        connected_ = initialize_connection_();
        if (!connected_) {
            throw std::runtime_error("Unable to connect to RabbitMQ");
        }
        return connected_;
    }

private:
    amqp_connection_state_t connection_state_ = nullptr;
    bool connected_ = false;

    bool initialize_connection_();
    bool connect_to_rabbitmq_(
        const std::string& hostname, int port, const std::string& username,
        const std::string& password
    );

    RabbitMQConnectionManager() = default;
};

} // namespace rabbitmq
} // namespace nutc
