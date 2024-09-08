#!/bin/bash

npm i

# Starting local s3 and postgres instances

docker-compose -f ./dev/docker-compose.yml up -d

# Creating nutc bucket in s3

aws --endpoint-url=http://localhost:4566 s3 mb s3://nutc
aws --endpoint-url=http://localhost:4566 s3api put-bucket-cors --bucket nutc --cors-configuration file://./dev/cors.json

# Generating prisma schema and migrating to postgres

npx prisma generate

source .env
DOCKER_POSTGRES_URL=$(echo $PRISMA_DATABASE_URL | sed 's/\(@\)postgres\(.\)/\1localhost\2/')
PRISMA_DATABASE_URL=$DOCKER_POSTGRES_URL npx prisma migrate dev

#aws --endpoiunt-url=http://localhost:4566 s3 ls s3://nutc --recursive
