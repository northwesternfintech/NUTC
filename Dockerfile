FROM conanio/gcc10

WORKDIR /app

RUN sudo apt-get update && sudo apt-get install -y cmake python3 python3-dev

COPY ./conanfile.py ./conanfile.py
COPY ./resources/cpp20 ./cpp20

RUN mkdir ./build && cd ./build && conan install .. -s build_type=Debug -b missing -pr ../cpp20 -pr:b ../cpp20

COPY ./src ./src
COPY ./CMakeLists.txt ./CMakeLists.txt
COPY ./CMakePresets.json ./CMakePresets.json

WORKDIR /app

RUN cmake --preset=dev
RUN cmake --build

# CMD ["./my_executable", "--uid"]
