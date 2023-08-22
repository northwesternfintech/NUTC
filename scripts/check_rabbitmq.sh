#!/bin/bash

USERNAME="NUFT"
PASSWORD="ADMIN"

if ! docker ps | grep -q nutc-rabbitmq-server; then
    echo "Starting RabbitMQ container..."
    docker run -d \
        --name nutc-rabbitmq-server \
        -p 5672:5672 \
        -p 15672:15672 \
        -e RABBITMQ_DEFAULT_USER=$USERNAME \
        -e RABBITMQ_DEFAULT_PASS=$PASSWORD \
        rabbitmq:management
    echo "RabbitMQ container started with username: $USERNAME and password: $PASSWORD."
    sleep 2
else
    echo "'nutc-rabbitmq-server' container is already running."
fi
