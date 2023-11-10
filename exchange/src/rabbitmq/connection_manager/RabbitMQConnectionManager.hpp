#pragma once

#include "client_manager/client_manager.hpp"

#include <string>

#include <rabbitmq-c/amqp.h>

namespace nutc {
namespace rabbitmq {

class RabbitMQConnectionManager {
public:
    // Delete the copy constructor and assignment operator
    RabbitMQConnectionManager(const RabbitMQConnectionManager&) = delete;
    RabbitMQConnectionManager& operator=(const RabbitMQConnectionManager&) = delete;

    // Delete the move constructor and assignment operator
    RabbitMQConnectionManager(RabbitMQConnectionManager&&) = delete;
    RabbitMQConnectionManager& operator=(RabbitMQConnectionManager&&) = delete;

    // Public method to access the singleton instance
    static RabbitMQConnectionManager&
    getInstance()
    {
        static RabbitMQConnectionManager instance;
        return instance;
    }

    bool connectToRabbitMQ(
        const std::string& hostname, int port, const std::string& username,
        const std::string& password
    );
    void closeConnection(const manager::ClientManager& client_manager);
    bool connectedToRMQ();
    amqp_connection_state_t get_connection_state();

private:
    amqp_connection_state_t connection_state;
    bool connected;

    bool initializeConnection();

    RabbitMQConnectionManager()
    {
        connection_state = amqp_new_connection();
        connected = initializeConnection();
    }
};

} // namespace rabbitmq
} // namespace nutc
