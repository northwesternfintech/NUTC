#pragma once

#include "market_order.hpp"

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
    void closeConnection();

private:
    amqp_connection_state_t conn;
    bool publishMessage(const std::string& queueName, const std::string& message);
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
