#!/bin/bash

wait_for_rabbitmq() {
    local pid_path="/var/lib/rabbitmq/mnesia/rabbit@$(hostname).pid"
    if [[ "$(uname)" == "Darwin" ]]; then
        pid_path="/usr/local/var/lib/rabbitmq/mnesia/rabbit@$(hostname).pid"
    fi
    echo "Waiting for RabbitMQ to be ready..."
    rabbitmqctl wait "$pid_path"
    echo "RabbitMQ is ready."
}

USERNAME="NUFT"
PASSWORD="ADMIN"

if docker ps | grep -q nutc-rabbitmq-server; then
    true
elif docker ps -a | grep -q nutc-rabbitmq-server; then
    echo "Starting the existing 'nutc-rabbitmq-server' container..."
    docker start nutc-rabbitmq-server
    echo "'nutc-rabbitmq-server' container started."
    wait_for_rabbitmq
else
    echo "Starting RabbitMQ container..."
    docker run -d \
        --name nutc-rabbitmq-server \
        -p 5672:5672 \
        -p 15672:15672 \
        -e RABBITMQ_DEFAULT_USER=$USERNAME \
        -e RABBITMQ_DEFAULT_PASS=$PASSWORD \
        rabbitmq:management
    echo "RabbitMQ container started with username: $USERNAME and password: $PASSWORD."
    wait_for_rabbitmq
fi



