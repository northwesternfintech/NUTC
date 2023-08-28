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
    bool initializeConnection(const std::string& queueName);
    bool publishMarketOrder(const MarketOrder& order);
    bool publishInit(const std::string& uid, bool ready);
    void closeConnection();

private:
    amqp_connection_state_t conn;
    bool publishMessage(const std::string& queueName, const std::string& message);
    bool initializeQueue(const std::string& queueName);
    bool connectToRabbitMQ(
        const std::string& hostname,
        int port,
        const std::string& username,
        const std::string& password
    );

    std::string consumeMessage(const std::string& queueName);
};

} // namespace rabbitmq
} // namespace nutc
