#!/bin/bash

# Init submodules
git submodule init

# Update submodules
git submodule update

# Start the vagrant box 'dut' with a freeflow shared folder
# mounted at '/freeflow'.
./start_vagrant.sh