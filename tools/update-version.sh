#!/bin/bash

# Get the version from CMake output
root="$(cd "$(dirname "$0")" && pwd)/.."
version=$(cmake ${root} 2>&1 | grep "LibrimeQjs version:" | awk '{print $4}')
gitref=$(git describe --always)

if [ -z "$version" ]; then
    echo "Error: Could not extract version number"
    exit 1
fi

if [ "$1" = "Nightly" ]; then
    version="${version}+${gitref}"
fi

# Update version in rime.d.ts
sed -i '' "s/LIB_RIME_QJS_VERSION/$version/" contrib/rime.d.ts

echo "Updated version to $version in rime.d.ts"
