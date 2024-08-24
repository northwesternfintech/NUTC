# BUILDING.md

## Prerequisites

Before building the project, ensure the following tools are installed on your system:

- go-task
- CMake
- Clang
- Conan
- Any additional tools as required by your environment

## Setup

1. **Detect Conan Profile:**
   Run the following command to detect your Conan profile:
   conan profile detect

2. **Configure Conan Profile for C++23:**
   Paste the following configuration into `$HOME/conan2/profiles/cpp23`:
   ```
   [settings]
   arch=x86_64
   build_type=Release
   compiler=gcc
   compiler.cppstd=gnu23
   compiler.libcxx=libstdc++11
   compiler.version=14.1
   os=Linux
   ```

## Build Instructions

1. **Install Dependencies:**
   Run the following task to install the necessary dependencies:
   task deps

2. **Initialize the Project:**
   Initialize the project by running:
   task init

3. **Build the Project:**
   To build the project, run:
   task build

   Alternatively, you can run the tests with:
   task test

## Mac Users
You must have xcode installed for cpp23 support.
