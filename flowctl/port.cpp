// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "port.hpp"
#include "util.hpp"

#include "freeflow/json.hpp"

#include <iostream>


using namespace ff;

constexpr int MAX_OPTIONS_LENGTH = 128;


// Bind the port with the given interface name to the specified 
// data plane.
//
//    add <dp-name> <interface>
//
// TODO: Can we add a logical interface to a port?
//
// TODO: We need serious clarification about what a port-id
// actually is. Is it like eth0? What are netmap ports? What
// about WiFi ports? Should we be checking validity in the CLI?
int 
add_port(int argc, char** argv) 
{ 
  if (!check_argument_arity_at_least(argc, 4))
    return -1;
  char const* dp = argv[1];
  char const* type = argv[2];
  char const* device = argv[3];
  char const* options;

  // Get options if present.
  if (argc > 4) {
    std::string buff;
    for (int i = 4; i < argc; i++)
      buff += argv[i];
    options = buff.c_str();
  }
  else
    options = "";

  std::string json = json::make_object({
    {"target",  "port"},
    {"command", "add"},
    {"dp",      dp},
    {"type",    type},
    {"device",  device},
    {"options", options}
  });
  return send_command(json);
}


// Unbind the port with the given port id from the 
//
//    del <port-id>
//
// The port-id can be the name of a physical interface or a
// scoped reference to the port.
int 
del_port(int argc, char** argv) 
{ 
  if (!check_argument_arity(argc, 3))
    return -1;
  char const* dp = argv[1];
  char const* port = argv[2];

  std::string json = json::make_object({
    {"target",  "port"},
    {"command", "del"},
    {"dp",      dp},
    {"pid",     port},
  });
  return send_command(json);
}


// Set a configuration value or modify the state of the port.
//
// Enable or disable the port and its various modes of
// operation. 
//
//    set <port-id> up
//    set <port-id> down
//    set <port-id> [no-]stp
//    set <port-id> [no-]receive
//    set <port-id> [no-]receive-stp
//    set <port-id> [no-]forward
//    set <port-id> [no-]flood
//    set <port-id> [no-]packet-in
//
// TODO: What other properties do we want to have to manage
// ports in a data plane?
int
set_port(int argc, char** argv)
{
  if (!check_argument_arity_at_least(argc, 3))
    return -1;
  char const* port = argv[1];

  if (argc == 3) {
    char const* state = argv[2];

    std::string json = json::make_object({
      {"target",  "port"},
      {"command", "mod"},
      {"port",    port},
      {"state",   state}
    });
    return send_command(json);
  } else {
    char const* prop = argv[2];
    char const* value = argv[3];

    std::string json = json::make_object({
      {"target",    "port"},
      {"command",   "set"},
      {"port",       port},
      {"property",   prop},
      {"value",      value}
    });
    return send_command(json);
  }
}


// Show the static configuration of a port.
//
//    show <port-id> 
int 
show_port(int argc, char** argv) 
{ 
  if (!check_argument_arity(argc, 2))
    return -1;
  char const* port = argv[1];

  std::string json = json::make_object({
    {"target",  "port"},
    {"command", "show"},
    {"port",     port}
  });
  return send_command(json);
}


// Request statistics for a port.
//
//    stat <port-id>
int 
stat_port(int argc, char** argv)
{
  if (!check_argument_arity(argc, 2))
    return -1;
  char const* port = argv[1];

  std::string json = json::make_object({
    {"target",  "port"},
    {"command", "stat"},
    {"port",    port}
  });
  return send_command(json);
}



// List the ports bound to the data plane name.
//
//    list <dp-name>
int 
list_ports(int argc, char** argv) 
{ 
  if (!check_argument_arity(argc, 2))
    return -1;
  char const* dp = argv[1];

  std::string json = json::make_object({
    {"target",  "port"},
    {"command", "list"},
    {"dp",      dp}
  });
  return send_command(json);
}


namespace
{

Command_map commands {
  {"add",   add_port},
  {"del",   del_port},
  {"set",   set_port},
  {"show",  show_port},
  {"stat",  stat_port},
  {"list",  list_ports}
};


} // namespace


// Execute a command in the dataplane scope. All such commands
// begin with:
//
//    port <command-name> [command-args]
int
port_command(int argc, char** argv)
{
  return execute_command(commands, argc, argv);
}

