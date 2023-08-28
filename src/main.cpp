#include "client/client.hpp"
#include "config.h"
#include "lib.hpp"
#include "logging.hpp"
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

    int num_clients = nutc::client::spawn_all_clients();
    nutc::logging::init(quill::LogLevel::TraceL3);

    if (num_clients == 0) {
        log_c(client_spawning, "Spawned 0 clients");
        return 1;
    };

    conn.wait_for_clients(num_clients);
    std::variant<rmq::InitMessage, rmq::MarketOrder, rmq::RMQError> mess =
        conn.consumeMessage();
    if (std::holds_alternative<rmq::MarketOrder>(mess)) {
        std::string buffer;
        rmq::MarketOrder order = std::get<rmq::MarketOrder>(mess);
        glz::write<glz::opts{.prettify = true}>(order, buffer);
        log_i(main, "Received market order: {}", buffer);
    }
    conn.closeConnection();

    return 0;
}
