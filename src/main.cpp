#include "client_manager/manager.hpp"
#include "config.h"
#include "firebase/firebase.hpp"
#include "lib.hpp"
#include "logging.hpp"
#include "matching/engine.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/rabbitmq.hpp"

#include <iostream>
#include <string>

#include <rabbitmq-c/amqp.h>

namespace rmq = nutc::rabbitmq;

rmq::RabbitMQ conn;
nutc::manager::ClientManager users;

void
handle_sigint(int sig)
{
    log_i(rabbitmq, "Caught SIGINT, closing connection");
    conn.closeConnection(users);
    sleep(1);
    exit(sig);
}

int
main()
{
    auto const lib = library{};
    signal(SIGINT, handle_sigint);

    if (!conn.initializeConnection()) {
        log_e(rabbitmq, "Failed to initialize connection");
        return 1;
    }

    glz::json_t::object_t firebase_users = nutc::client::get_all_users();
    users.initialize_from_firebase(firebase_users);
    int num_clients = nutc::client::spawn_all_clients(users);

    nutc::logging::init(quill::LogLevel::TraceL3);

    if (num_clients == 0) {
        log_c(client_spawning, "Spawned 0 clients");
        return 1;
    };

    nutc::matching::Engine engine;

    conn.waitForClients(num_clients, users);
    conn.handleIncomingMessages(users, engine);
    conn.closeConnection(users);

    return 0;
}
