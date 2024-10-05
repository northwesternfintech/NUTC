#!/bin/bash

set -e

conan profile detect -f

std=23
version=14.2
  

profile="$(conan profile path default)"

mv "$profile" "${profile}.bak"
sed -e 's/^\(compiler\.cppstd=\).\{1,\}$/\1'"$std/" \
    "${profile}.bak" > "$profile"
rm "${profile}.bak"
