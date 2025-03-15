#!/bin/bash

# usage:
# lint - `bash tools/clang-format.sh lint`
# format - `bash tools/clang-format.sh format`

root="$(cd "$(dirname "$0")" && pwd)/.."

options="-Werror --dry-run" # default to lint only mode

if [ "$1" = "format" ]; then
    options="-Werror -i"
fi

## run on a file to validate the configuration of .clang-format
clang-format ${options} ${root}/src/module.cc

find ${root}/src \
    -type f \( -name '*.h' -o -name '*.hpp' -o -name '*.cc' -o -name '*.cpp' \) | \
    xargs clang-format ${options} || { echo Please lint your code by '"'"bash ./tools/clang-format.sh lint"'"'.; false; }

find ${root}/tests \
    -type f \( -name '*.h' -o -name '*.hpp' -o -name '*.cc' -o -name '*.cpp' \) | \
    xargs clang-format ${options} || { echo Please lint your code by '"'"bash ./tools/clang-format.sh lint"'"'.; false; }
