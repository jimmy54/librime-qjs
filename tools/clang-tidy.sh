#!/bin/bash

# usage:
# - lint the modified files: `bash tools/clang-tidy.sh modified`
# - lint all the files: `bash tools/clang-tidy.sh all`

root="$(cd "$(dirname "$0")" && pwd)/.."

mode="modified"
if [ "$1" = "all" ]; then
    mode="all"
fi

cmake ${root} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

mv ${root}/compile_commands.json ${root}/build

options="-p ${root}/build \
    --config-file=${root}/.clang-tidy \
    --warnings-as-errors=* \
    --header-filter=\"${root}/(src|tests)/.*\" \
    --system-headers=0 \
    --use-color \
    -extra-arg=-I${root}/src \
    -extra-arg=-I${root}/src/engines \
    -extra-arg=-I${root}/src/gears \
    -extra-arg=-I${root}/src/types \
    -extra-arg=-I${root}/src/misc \
    -extra-arg=-I${root}/src/patch/quickjs \
    -extra-arg=-I${root}/tests \
    -extra-arg=-isystem${root}/../../src \
    -extra-arg=-isystem${root}/../../build/src \
    -extra-arg=-isystem${root}/../../include \
    -extra-arg=-isystem${root}/../../include/glog \
    -extra-arg=-isystem${root}/thirdparty/quickjs \
    -extra-arg=-isystem/usr/local/include \
    -extra-arg=-stdlib=libc++ \
    -extra-arg=-D_ENABLE_JAVASCRIPTCORE \
    -extra-arg=-D_GNU_SOURCE \
    -extra-arg=-DGLOG_EXPORT=__attribute__((visibility(\"default\"))) \
    -extra-arg=-DGLOG_NO_EXPORT=__attribute__((visibility(\"default\"))) \
    -extra-arg=-DGLOG_DEPRECATED=__attribute__((deprecated))"

ignore_files="test_switch.h"

process_file() {
    if [[ $1 =~ $ignore_files ]]; then
        echo "Ignoring $1..."
        return
    fi
    echo "Processing $1..."
    clang-tidy ${options} "$1"
}

export -f process_file
export options
export ignore_files

if [ "$mode" = "all" ]; then
    echo "Linting all files..."

    find ${root}/{src,tests} \
        -type f \( -name '*.h' -o -name '*.hpp' -o -name '*.c' -o -name '*.cc' -o -name '*.cpp' \) | \
        xargs -P $(sysctl -n hw.ncpu) -I {} bash -c 'process_file "{}"'
else
    echo "Linting modified files..."

    git diff --name-only HEAD | \
        grep -E '\.(cpp|cc|c|h|hpp)$' | \
        xargs -P $(sysctl -n hw.ncpu) -I {} bash -c 'process_file "{}"'
fi
