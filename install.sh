#!/bin/bash

# Check if running as root
if [ "${EUID}" -ne 0 ]; then
    echo -e "This script must be run as root. It's okay,\\n > Consider run \\"sudo ./install.sh\\" instead." >&2
    exit 1
fi

# Build the project
cmake -S . -B build
cmake --build build

# Install the executable to /usr/local/bin
install -m 755 build/the-mee-null /usr/local/bin/

# Verify installation
if which the-mee-null &>/dev/null; then
    echo "Installation successful. 'the-mee-null' is now available."
else
    echo "Error: Installation failed. 'the-mee-null' was not installed in /usr/local/bin." >&2
    exit 1
fi
