#pragma once

#include "glz_templates.hpp"

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
    bool publishInit(const std::string& uid, bool ready);
    std::function<bool(const std::string&, float, bool, const std::string&, float)>
    getMarketFunc();
    void closeConnection();

private:
    amqp_connection_state_t conn;
    bool publishMessage(const std::string& queueName, const std::string& message);
    bool initializeQueue(const std::string& queueName);
    bool publishMarketOrder(
        const std::string& security,
        float quantity,
        bool side,
        const std::string& type,
        float price
    );

    bool connectToRabbitMQ(
        const std::string& hostname,
        int port,
        const std::string& username,
        const std::string& password
    );

    std::string consumeMessageAsString(const std::string& queueName);
    std::variant<ShutdownMessage, RMQError> consumeMessage(const std::string& queueName
    );
};

} // namespace rabbitmq
} // namespace nutc
