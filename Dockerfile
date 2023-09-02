FROM ubuntu:latest

WORKDIR /app
RUN apt-get update && apt-get install -y python3 python3-dev
RUN apt-get install -y ca-certificates

COPY ./build/dev/NUTC-client ./NUTC
RUN chmod +x ./NUTC

CMD ./NUTC
