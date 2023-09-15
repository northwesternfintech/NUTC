#pragma once

#include "glz_templates.hpp"

#include <unistd.h>

#include <iostream>
#include <string>

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

namespace nutc {
namespace rabbitmq {

class RabbitMQ {
public:
    RabbitMQ(const std::string& uid);
    bool initializeConnection(const std::string& queueName);
    bool initializeConsume(const std::string& queueName);
    bool publishInit(const std::string& uid, bool ready);
    std::function<bool(const std::string&, const std::string&, const std::string&, float, float)>
    getMarketFunc(const std::string& uid);
    std::variant<ShutdownMessage, RMQError> handleIncomingMessages();
    void closeConnection();

private:
    amqp_connection_state_t conn;
    bool publishMessage(const std::string& queueName, const std::string& message);
    bool initializeQueue(const std::string& queueName);
    bool publishMarketOrder(
        const std::string& client_uid,
        const std::string& side,
        const std::string& type,
        const std::string& ticker,
        float quantity,
        float price
    );

    bool connectToRabbitMQ(
        const std::string& hostname,
        int port,
        const std::string& username,
        const std::string& password
    );

    std::string consumeMessageAsString();
    std::variant<ShutdownMessage, RMQError, ObUpdate> consumeMessage();
};

} // namespace rabbitmq
} // namespace nutc
