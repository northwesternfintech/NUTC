#build stage
FROM python:3.12 as build

RUN pip install conan numpy pandas polars scipy scikit-learn sortedcontainers lightgbm\
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

ENV NUTC_LINTER_SPAWNER_BINARY_PATH="/app/exchange/build/LINTER_spawner"

RUN chmod +x /app/exchange/build/LINTER
RUN chmod +x /app/exchange/build/LINTER_spawner

COPY ./exchange/template.cpp /app/template.cpp
ENV NUTC_CPP_TEMPLATE_PATH="/app/template.cpp"

EXPOSE 18081

CMD /app/exchange/build/LINTER
