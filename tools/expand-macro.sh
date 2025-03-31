#!/bin/bash

## usage: `bash ./tools/expand-macro.sh src/types/qjs_candidate.cc`

root="$(cd "$(dirname "$0")" && pwd)/.."

target="${root}/build/expanded"
mkdir -p "${target}"

file=$1
filename=$(basename "$file")

/opt/local/libexec/llvm-20/bin/clang++ \
  -E -nostdinc++ \
  -isystem /opt/local/libexec/llvm-20/include/c++/v1 \
  -isystem /opt/local/libexec/llvm-20/lib/clang/20/include \
  -isystem /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include \
  -I${root}/src \
  -I${root}/src/gears \
  -I${root}/src/helpers \
  -I${root}/src/types \
  -I${root}/../../src \
  -I${root}/../../build/src \
  -I${root}/../../include \
  -I${root}/../../include/glog \
  -I${root}/thirdparty/quickjs \
  -stdlib=libc++ \
  -D_GNU_SOURCE \
  -DGLOG_EXPORT=__attribute__\(\(visibility\(\"default\"\)\)\) \
  -DGLOG_NO_EXPORT=__attribute__\(\(visibility\(\"default\"\)\)\) \
  -DGLOG_DEPRECATED=__attribute__\(\(deprecated\)\) \
  ${root}/${file} \
  -o ${target}/${filename}.i &&
  echo "expanded the macros of ${file} to ${target}/${filename}.i"
