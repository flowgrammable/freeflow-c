
# nomg

nomg is the [noproto](/) data plane instance manager. Users are able to manage
data plane instances via [noctl](/noctl)'s CLI. nomg will receive commands in a
synchronous fashion and process them in the order they are received. Data plane
instances will run as child processes.

FIXME: Most of this is dated... we need to sync the docs with the actual
implementation.

## Process Management

Since nomg will be managing several data planes, there are necessary bookkeeping
data structures that must be provided.

### Data Plane Table

A listing of the current data plane instances that are running with their
associated data. The working form of this table follows.

|Name | PID | Type | ... |
|-----|-----|------|-----|

The label for the data plane | The Process (Plane) ID | The data plane
architecture and application | ...

## Port Management

In order to supply ports to active data planes, nomg will require a simple table
to keep track of port assignments.

### Port Table

A table for data plane instance labels and the ports they are using. For now
each port will occupy one row in the table.

Name | Port
-----|-----

The label for the data plane | The port number in use

## Data Plane Management

nomg will use the working list of functional commands that follow to manage data
plane instances.

`add-dp <dp name> <dp type>`
- Creates a new instance of the given data plane type
```
nomg_add_dp(name, type){
    if (name doesn't exist){
        PID = fork()
        if child then exec(type)
        else{
            update dp_table(name, PID, type)
            return success;
        }
    }
    return name_not_found_error;
}
```

`add-port <dp name> <port>`
- Adds a port to a current data plane instance with matching name
```
nomg_add_port(name, port){
    if(name exists){
        if(port isn't being used){
            add port to data plane(name, port)
            update port_table(name, port)
            return success;
        }
        else return port_in_use_error;
    }
    else return name_not_found_error;
}
```

`del-dp <dp name>`
- Removes the data plane instance with matching name
```
nomg_del_dp(name){
    if(name exists){
        lookup PID(name)
        kill PID
        update dp_table(name)
        return success;
    }
    else return name_not_found_error;
}
```

`del-port <dp name> <port>`
- Removes the port given from the data plane instance with matching name
```
nomg_del_port(name, port){
    if(name exists){
        if(port is being used by name){
            remove port from data plane(name)
            update port_table(name, port)
            return success;
        }
        else return port_not_in_use_error;
    }
    else return name_not_found_error;
}
```

`list-dps`
- Lists all active data planes
```
nomg_list_dps(void){
    return dp_table;
}
```

`list-ports <dp name>`
- Lists all ports being used by the data plane instance with matching name
```
nomg_list_ports(name){
    if(name exists){
        return list of ports used by name;
    }
    else return name_not_found_error;
}
```

`show-dp <dp name>`
- Sends a command to the data plane as a query
```
nomg_show_dp(name){
    if(name exists){
        forward cmd to data plane as query
        return response;
    }
    else return name_not_found_error;
}
```

`show-port <dp name> <port>`
- Sends a command to the data plane as a query
```
nomg_show_port(name, port){
    if(name exists){
        if(port is being used by name){
            forward cmd to data plane as query
            return response;
        }
        else return port_not_in_use_error;
    }
    else return name_not_found_error;
}
```

`nomg load-app <dp name> <app name>`

`nomg unload-app <dp name> <app name>`

`nomg start-app <dp name> <app name>`

`nomg stop-app <dp name> <app name>`
