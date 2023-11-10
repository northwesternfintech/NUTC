#should be run from root:
#sudo docker build -t nutc24:latest . -f exchange/Dockerfile

# Build stage for wrapper
FROM python:3.11-slim as build-wrapper

RUN pip install conan numpy pandas polars scipy scikit-learn \
    # c++ stuff
    && apt update \
    && apt install -y --no-install-recommends build-essential libssl-dev cmake git

WORKDIR /app/wrapper
COPY ./wrapper/conanfile.py /app/wrapper
COPY ./.github/scripts/conan-profile.sh /app/wrapper

RUN cat conan-profile.sh | bash \
    && conan install . -b missing

COPY ./wrapper /app/wrapper
RUN cmake --preset=ci-docker \
    && cmake --build build --config Release -j

# Build stage for exchange

FROM python:3.11-slim as build-exchange

RUN pip install conan numpy pandas polars scipy scikit-learn \
    # c++ stuff
    && apt update \
    && apt install -y --no-install-recommends build-essential libssl-dev cmake git

WORKDIR /app/exchange
COPY ./exchange/conanfile.py /app/exchange
COPY ./.github/scripts/conan-profile.sh /app/exchange

RUN cat conan-profile.sh | bash \
    && conan install . -b missing

COPY ./exchange /app/exchange
RUN cmake --preset=ci-docker \
    && cmake --build build --config Release -j

# Main stage for exchage
FROM python:3.11-slim as run-exchange

RUN apt update
RUN apt install -y rabbitmq-server
RUN pip install numpy pandas polars scipy scikit-learn

COPY --from=build-wrapper /app/wrapper/build/NUTC-client /bin/NUTC-client
COPY --from=build-exchange /app/exchange/build/NUTC24 /bin/NUTC-exchange
COPY ./exchange/scripts/docker-start.sh /app/docker-start.sh

RUN chmod +x /bin/NUTC-client
RUN chmod +x /bin/NUTC-exchange
RUN chmod +X /app/docker-start.sh

ENV RABBITMQ_DEFAULT_USER="NUFT"
ENV RABBITMQ_DEFAULT_PASS="ADMIN"

ENTRYPOINT ["./app/docker-start.sh"]
