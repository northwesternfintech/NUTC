FROM python:3.12 as build

RUN pip install conan numpy pandas polars scipy sortedcontainers scikit-learn lightgbm \
    # c++ stuff
    && apt update \
    && apt install -y --no-install-recommends build-essential libssl-dev cmake git

WORKDIR /app/exchange
COPY ./exchange/conanfile.py /app/exchange/conanfile.py
COPY ./.github/scripts/conan-profile.sh /app/exchange

RUN cat conan-profile.sh | bash \
    && conan install . -b missing

COPY ./exchange/src /app/exchange/src
COPY ./exchange/CMakeLists.txt /app/exchange
COPY ./exchange/CMakePresets.json /app/exchange
COPY ./exchange/cmake /app/exchange/cmake
COPY ./exchange/test /app/exchange/test

ARG firebase_emulator=false

RUN if [ "$firebase_emulator" = "false" ]; then \
    cmake --preset=ci-docker; \
    else \
    cmake --preset=ci-docker -DLOCAL_DEV=ON; \
    fi

RUN cmake --build build --config Release -j

RUN mv /app/exchange/build/NUTC /usr/bin/NUTC-exchange
RUN mv /app/exchange/build/WRAPPER /usr/bin/WRAPPER

# Todo: make default?
COPY ./exchange/config.yml /app/exchange/config.yml

RUN chmod +x /usr/bin/NUTC-exchange /usr/bin/WRAPPER

COPY ./exchange/template.cpp /app/template.cpp

ENV NUTC_WRAPPER_BINARY_PATH="/usr/bin/WRAPPER"
ENV NUTC_CONFIG_FILE="/app/exchange/config.yml"
ENV NUTC_CPP_TEMPLATE_PATH="/app/template.cpp"

EXPOSE 4152 18080

ENTRYPOINT ["/usr/bin/NUTC-exchange"]
