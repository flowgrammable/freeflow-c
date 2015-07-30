# Freeflow
A highly configurable and programmable SDN runtime.

## Getting started with the Freeflow development environment.
1. Install [virtual box](virtualbox.org) and [vagrant](vagrantup.com).
2. Clone the freeflow repo.
```sh
$ git clone git@github.com:flowgrammable/freeflow.git
or
$ git clone https://github.com/flowgrammable/freeflow.git
```
3. Initialize the dev-machine submodule.
```sh
git submodule init && git submodule update
```
4. Move to the `dev-machine` directory and start the machine.
  - **Note**: to sync the freeflow folder on your host machine edit the `Vagrantfile` and follow the comments in the section labeled 'Share an additional folder to the guest VM'.
```sh
$ cd dev-machine && vagrant up dut
```

