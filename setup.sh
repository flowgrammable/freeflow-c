#!/bin/bash

# Init submodules
git submodule init

# Update submodules
git submodule update --init

# For submodules tracking certain commits
git submodule update --remote

# Start the vagrant box 'dut' with a freeflow shared folder
# mounted at '/freeflow'.
./start_vagrant.sh