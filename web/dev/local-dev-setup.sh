#!/bin/bash

# docker-compose -f ./dev/docker-compose.yml down -t 1

# npm i

# Starting local s3 and postgres instances

# docker-compose -f ./dev/docker-compose.yml up -d

# Creating nutc bucket in s3
sleep 1
aws --endpoint-url=http://localstack:4566 s3 mb s3://nutc
sleep 1
aws --endpoint-url=http://localstack:4566 s3api put-bucket-cors --bucket nutc --cors-configuration file:///home/cors.json

