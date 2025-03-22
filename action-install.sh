#!/bin/bash

# This script installs the dependencies to build the project within libRime
git submodule update --init --recursive

if [[ "$OSTYPE" == "darwin"* ]]; then
    brew install nodejs
elif [[ "$OSTYPE" == "linux-gnu"* ]] && [ -f /etc/lsb-release ]; then
    sudo apt-get update
    sudo apt-get install -y nodejs
fi


cd tests/js
npm install
