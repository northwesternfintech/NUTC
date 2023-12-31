
# Building NUTC Guide
## Dependencies
1. Conan
- `pip install conan`
- `dnf install cmake`
- `dnf install go-task`
- `dnf install perl`
- `conan profile detect`
- Copy `.conan2/profiles/default` to `.conan2/profiles/cpp20` 
- Change compiler.cppstd from gnu14 to gnu20
- Change compiler.version from 8 to 13
2. Install dependencies
- `mkdir build && cd build`
- `conan install .. -s build_type=Debug -b missing -pr cpp20 -pr:b cpp20`
3. Change `CMakeUserPresets.json` the following:
```
{
  "version": 2,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 14,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "dev",
      "binaryDir": "${sourceDir}/build/dev",
      "inherits": ["dev-mode", "conan", "ci-darwin"],
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "dev",
      "configurePreset": "dev",
      "configuration": "Debug"
    }
  ],
  "testPresets": [
    {
      "name": "dev",
      "configurePreset": "dev",
      "configuration": "Debug",
      "output": {
        "outputOnFailure": true
      }
    }
  ]
}
```
4. `cd .. && go-task init`


## Building
`go-task build`


## Running
1. NUTC-Client
- Standard setup instructions included in repo
- Must run `go-task move-to-bin`
2. Docker
- Must have docker installed and running
3. `go-task run`

## Documentation
1. Install doxygen
2. `doxygen Doxyfile`
3. `open html/index.html`
