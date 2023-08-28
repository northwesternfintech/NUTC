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
    bool publishMarketOrder(
        const std::string& security, int quantity, bool side, const std::string& type
    );
    bool publishInit(const std::string& uid, bool ready);
  std::function<bool(const std::string&, int, bool, const std::string&)> getMarketFunc();
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
