
# Building NUTC Guide
## Dependencies
1. Conan
- `pip install conan`
- `conan profile detect`
- Copy .conan2/default to .conan2/cpp20 
- Change gnu17 to gnu20 
2. Install dependencies
- `mkdir build && cd build`
- `conan install .. -s build_type=Debug -b missing -pr cpp20 -pr:b cpp20`
3. Setup CMakeUserPresets.txt
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
4. `cmake --preset=dev`


## Building
`cmake --build --preset=dev`

