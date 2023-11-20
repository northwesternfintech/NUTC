#!/bin/sh
rabbitmq-server -detached
sleep 5
exec NUTC-exchange "$@"
