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

RUN cmake --preset=ci-docker

RUN cmake --build build --config Release -j

RUN mv /app/exchange/build/NUTC /app/NUTC
RUN mv /app/exchange/build/WRAPPER /app/WRAPPER

# Todo: make default?
COPY ./exchange/config.yml /app/exchange/config.yml

RUN chmod +x /app/NUTC /app/WRAPPER

COPY ./exchange/template.cpp /app/template.cpp

ENV NUTC_WRAPPER_BINARY_PATH="/app/WRAPPER"
ENV NUTC_CONFIG_FILE="/app/exchange/config.yml"
ENV NUTC_CPP_TEMPLATE_PATH="/app/template.cpp"

EXPOSE 4152 18080

ENTRYPOINT ["/app/NUTC"]
