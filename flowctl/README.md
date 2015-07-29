# flowctl

flowctl is the client side CLI for interacting with [flowmgr](/flowmgr). It
accepts commands and arguments from standard input and ensures that they are
well-formed before sending them for processing.


## Arguments
Below is a working list of valid arguments for flowctl


### Argument Listing

Arg Name | Arg Type | Definition
---------|----------|-----------
DP Name  | String   | a name is used to label a data plane
DP Type  | String   | a type defines the architecture and application for a data plane
Port     | Int      | a port number


## Commands

Below is a working list of flowctl commands, their form, and any other 
relevent information


### Command Listing

`flowctl add-dp <dp name> <dp type>`
- Creates a data plane of given type with the given name

`flowctl add-port <dp name> <port>`
- Adds a port to an active data plane

`flowctl del-dp <dp name>`
- Destroys a data plane with the matching name

`flowctl del-port <dp name> <port>`
- Deletes a port from an active data plane

`flowctl list-dps`
- Lists all active data planes

`flowctl list-ports <dp name>`
- Lists all ports being used by the given data plane name

`flowctl show-dp <dp name>`
- Shows some information about a data plane

`flowctl show-port <dp name> <port>`
- Shows some information about a port in an active data plane 

`flowctl load-app <dp name> <app name>`

`flowctl unload-app <dp name> <app name>`

`flowctl start-app <dp name> <app name>`

`flowctl stop-app <dp name> <app name>`
