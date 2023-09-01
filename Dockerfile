FROM conanio/gcc10

WORKDIR /app

COPY ./conanfile.py ./conanfile.py
COPY ./CMakeLists.txt ./CMakeLists.txt
COPY ./src ./src
COPY ./CMakePresets.json ./CMakePresets.json

COPY ./resources/cpp20 ./cpp20

RUN mkdir ./build && cd ./build && conan install .. -s build_type=Debug -b missing -pr ../cpp20 -pr:b ../cpp20

WORKDIR /app

RUN cmake --preset=dev
RUN cmake --build

# CMD ["./my_executable", "--uid"]
