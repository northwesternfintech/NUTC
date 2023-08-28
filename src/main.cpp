#include "client/client.hpp"
#include "config.h"
#include "lib.hpp"
#include "logging.hpp"
#include "rabbitmq/rabbitmq.hpp"

#include <iostream>
#include <string>

#include <rabbitmq-c/amqp.h>

int
main()
{
    auto const lib = library{};
    nutc::logging::init(quill::LogLevel::TraceL3);

  nutc::rabbitmq::RabbitMQ conn;

    if (!conn.initializeConnection()) {
        log_e(rabbitmq, "Failed to initialize connection");
        return 1;
    }

    std::string mess = conn.consumeMarketOrder();
    log_i(rabbitmq, "Received message: {}", mess);
    conn.closeConnection();

    nutc::client::spawn_all_clients();

    return 0;
}
