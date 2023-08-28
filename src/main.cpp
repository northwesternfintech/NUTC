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

    nutc::rabbitmq::RabbitMQ conn;

    if (!conn.initializeConnection()) {
        log_e(rabbitmq, "Failed to initialize connection");
        return 1;
    }

    int num_clients = nutc::client::spawn_all_clients();
    nutc::logging::init(quill::LogLevel::TraceL3);

    if (num_clients == 0) {
        log_e(client_spawning, "Spawned 0 clients");
        return 1;
    };

    conn.wait_for_clients(num_clients);
    conn.closeConnection();

    return 0;
}
