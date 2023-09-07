#include "client/client.hpp"
#include "config.h"
#include "lib.hpp"
#include "logging.hpp"
#include "matching/engine.hpp"
#include "rabbitmq/rabbitmq.hpp"

#include <iostream>
#include <string>

#include <rabbitmq-c/amqp.h>

namespace rmq = nutc::rabbitmq;

int
main()
{
    auto const lib = library{};

    rmq::RabbitMQ conn;

    if (!conn.initializeConnection()) {
        log_e(rabbitmq, "Failed to initialize connection");
        return 1;
    }

    glz::json_t::object_t users = nutc::client::get_all_users();
    int num_clients = nutc::client::spawn_all_clients(users);

    nutc::logging::init(quill::LogLevel::TraceL3);

    if (num_clients == 0) {
        log_c(client_spawning, "Spawned 0 clients");
        return 1;
    };

    nutc::matching::Engine engine;

    conn.wait_for_clients(num_clients);
    conn.handle_incoming_messages(engine);
    conn.closeConnection();

    return 0;
}
