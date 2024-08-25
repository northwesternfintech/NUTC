#!/bin/bash

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
profile_path="$HOME/.conan2/profiles/cpp20"

if [ ! -f "$profile_path" ]; then
  profile_content="[settings]
  arch=x86_64
  build_type=Release
  compiler=gcc
  compiler.cppstd=gnu23
  compiler.libcxx=libstdc++11
  compiler.version=13
  os=Linux"

  mkdir -p "$(dirname "$profile_path")"
  echo "$profile_content" > "$profile_path"

  echo "Profile cpp20 has been created in $profile_path"
fi

directories=("exchange" "linter")

for dir in "${directories[@]}"; do
    presets_file="${script_dir}/../${dir}/CMakeUserPresets.json"
    if [ ! -f "$presets_file" ]; then
        default_presets="${script_dir}/DefaultCMakeUserPresets.json"
        cp "$default_presets" "$presets_file"
        echo "Created default user presets in $presets_file"
    fi
done
