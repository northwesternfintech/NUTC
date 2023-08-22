#!/bin/bash

# Check if redis is running, otherwise, start it
if ! docker ps | grep -q nutc-redis-server; then
    echo "Starting Redis container..."
    docker run -d --name nutc-redis-server -p 6379:6379 redis:latest
    echo "Redis container started."
else
    echo "Redis container is already running."
fi
