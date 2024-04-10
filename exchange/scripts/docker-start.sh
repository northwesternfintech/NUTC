#!/bin/sh
rabbitmq-server -detached
rabbitmqctl wait /var/lib/rabbitmq/mnesia/rabbit@$(hostname).pid
exec NUTC-exchange "$@"
