#pragma once

#include <string>

#include <rabbitmq-c/amqp.h>

namespace nutc {
namespace rabbitmq {

class RabbitMQConnectionManager {
    // Only allow the singleton instance to create the connection
    RabbitMQConnectionManager& operator=(RabbitMQConnectionManager&&) = default;

    // TODO(stevenewald): consider RAII?
    ~RabbitMQConnectionManager() = default;

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

    bool connect_to_rabbitmq(
        const std::string& hostname, int port, const std::string& username,
        const std::string& password
    );
    void close_connection();

    [[nodiscard]] bool
    connected_to_rabbitmq() const
    {
        return connected_;
    }

    amqp_connection_state_t
    get_connection_state()
    {
        return connection_state_;
    }

private:
    amqp_connection_state_t connection_state_;
    bool connected_;

    bool initialize_connection_();

    RabbitMQConnectionManager() :
        connection_state_(amqp_new_connection()), connected_(initialize_connection_())
    {}
};

} // namespace rabbitmq
} // namespace nutc
