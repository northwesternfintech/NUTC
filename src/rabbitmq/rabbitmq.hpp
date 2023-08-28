#pragma once

#include "market_order.hpp"

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

#include <string>
#include <iostream>

namespace nutc {
namespace rabbitmq {
bool connectToRabbitMQ(
    amqp_connection_state_t& conn, const std::string& hostname, int port,
    const std::string& username, const std::string& password
);

bool publishMessage(
    amqp_connection_state_t& conn, const std::string& queueName,
    const std::string& message
);

std::string consumeMessage(amqp_connection_state_t& conn, const std::string& queueName);

bool initializeConnection(amqp_connection_state_t& conn);

void closeConnection(amqp_connection_state_t& conn);

} // namespace rabbitmq
} // namespace nutc
