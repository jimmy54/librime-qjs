# This script installs the dependencies to build the project within libRime
git submodule update --init --recursive

choco install nodejs

cd tests/js
npm install
