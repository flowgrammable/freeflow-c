
# Flowmgr

Flowmgr is the [flowpath](https://github.com/flowgrammable/freeflow/flowpath) data plane instance manager. Users are able to access flowpath data plane instances using a [Flowmgr Client Interface](#client-interface).


## Client Interaces

Flowmgr is intended to allow multiple forms of communication for configuration and management of data plane instances. Below is additional information about these interfaces.

### CLI

[Flowctl](https://github.com/flowgrammable/freeflow/flowctl) allows a user to manipulate data plane instances through flowmgr from the command line using Unix Sockets and JSON style messages. See the flowctl project page for additional information.


### Flowsim

[Flowsim](https://github.com/flowgrammable/flowsim) is [Flowgrammable's](http://flowgrammable.org) OpenFlow data plane simulator. There are plans to set up communcation channels between Flowmgr and Flowsim to allow for configuration, management, as well as debugging functionality.


## Data Plane 

Flowmgrs main purpose is to configure and manage flowpath data plane instances. Below is additional information about the management and communcation protocols used.

### Management

Since there could possibly be multiple data plane instances running in [flowpath](https://github.com/flowgrammable/freeflow/flowpath) some back end store is needed. Flowmgr uses a simple chained hash table to keep track of data plane instances and their labels. 


### Communication

Flowmgr and [flowpath](https://github.com/flowgrammable/freeflow/flowpath) will also communicate over sockets but use a closed binary interface for relaying messages. The messages map to commands available to the manager for any data plan instance.