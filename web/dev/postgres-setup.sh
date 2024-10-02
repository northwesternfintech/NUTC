!/bin/bash
# Generating prisma schema and migrating to postgres

npx prisma generate

source .env
DOCKER_POSTGRES_URL=$(echo $PRISMA_DATABASE_URL | sed 's/\(@\)postgres\(.\)/\1localhost\2/')
PRISMA_DATABASE_URL=$DOCKER_POSTGRES_URL npx prisma migrate dev
