FROM python:3.12 as build

RUN pip install conan numpy pandas polars scipy scikit-learn \
    # c++ stuff
    && apt update \
    && apt install -y --no-install-recommends build-essential libssl-dev cmake git

WORKDIR /app/exchange
COPY ./exchange/conanfile_no_ncurses.py /app/exchange/conanfile.py
COPY ./.github/scripts/conan-profile.sh /app/exchange

RUN cat conan-profile.sh | bash \
    && conan install . -b missing

COPY ./exchange /app/exchange
RUN cmake --preset=ci-docker \
    && cmake --build build --config Release -j

# Main stage for exchange
FROM python:3.12 as run-exchange

RUN apt update \
    && apt install -y rabbitmq-server \
    && pip install numpy pandas polars scipy scikit-learn prophet attrs cppyy lightgbm xgboost 

COPY --from=build /app/exchange/config.yml .
COPY --from=build /app/exchange/build/NUTC /bin/NUTC-exchange
COPY --from=build /app/exchange/build/WRAPPER /bin/WRAPPER
COPY ./exchange/scripts/docker-start.sh /app/docker-start.sh

RUN chmod +x /bin/NUTC-exchange /bin/WRAPPER /app/docker-start.sh

ENV RABBITMQ_DEFAULT_USER="NUFT"
ENV RABBITMQ_DEFAULT_PASS="ADMIN"
ENV NUTC_WRAPPER_BINARY_PATH="/bin/WRAPPER"

ENTRYPOINT ["./app/docker-start.sh"]
