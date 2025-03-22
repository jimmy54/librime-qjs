#!/bin/bash

# This script installs the dependencies to build the project within libRime
git submodule update --init --recursive

if [[ "$OSTYPE" == "darwin"* ]]; then
    brew install nodejs
elif [[ "$OSTYPE" == "linux-gnu"* ]] && [ -f /etc/lsb-release ]; then
    sudo apt-get update
    sudo apt-get install -y nodejs
else
    echo "Error: NodeJS is required to run the unit tests."
    echo "Please install NodeJS for your operating system in 'action-install.sh'."
    exit 1
fi


cd tests/js
npm install
