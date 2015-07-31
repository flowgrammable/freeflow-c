#!/bin/bash

# Set the path to the freeflow repo on the host machine
# and start the 'dut' guest machine with a shared folder
# mounted at '/freeflow'.
CURR=$PWD
cd dev-machine
FF_PATH=$CURR vagrant up dut