#!/bin/bash

root="$(cd "$(dirname "$0")" && pwd)/.."

## format a file to validate the configuration of .clang-format
clang-format -i ${root}/src/module.cc

find ${root}/src \
    -type f \( -name '*.h' -o -name '*.hpp' -o -name '*.cc' -o -name '*.cpp' \) \
    -exec clang-format -i {} +

find ${root}/tests \
    -type f \( -name '*.h' -o -name '*.hpp' -o -name '*.cc' -o -name '*.cpp' \) \
    -exec clang-format -i {} +
