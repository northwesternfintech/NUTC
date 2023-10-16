#!/bin/bash

if docker ps | grep -q nutc-redis-server; then
    echo "Redis container is already running."
elif docker ps -a | grep -q nutc-redis-server; then
    echo "Starting the existing 'nutc-redis-server' container..."
    docker start nutc-redis-server
    echo "'nutc-redis-server' container started."
    sleep 5
else
    echo "Starting Redis container..."
    docker run -d --name nutc-redis-server -p 6379:6379 redis:latest
    echo "Redis container started."
    sleep 5
fi
