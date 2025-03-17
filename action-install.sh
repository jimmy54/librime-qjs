#!/bin/bash

# This script installs the dependencies to build the project within libRime
git submodule update --init --recursive

brew install nodejs

cd tests/js
npm install
