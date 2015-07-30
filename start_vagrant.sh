#!/bin/bash

# Set the vagrant box current working directory
export VAGRANT_CWD=$PWD/dev-machine

# Set the path to the freeflow repo on the host machine
# and start the 'dut' guest machine with a shared folder
# mounted at '/freeflow'.
FF_PATH=$PWD vagrant up dut