// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "util.hpp"

#include "freeflow/json.hpp"

#include <iostream>

#include <unistd.h>


using namespace ff;

// The global configuration.
extern Config cfg;


namespace
{

// FIXME: This should be set in the CMAKE configuration.
#ifdef __linux__
#  define FREEFLOW_EXT_SUFFIX ".so"
#elif __APPLE__
#  define FREEFLOW_EXT_SUFFIX ".dylib"
#else
#  error Extensions not enabled for platform
# endif

// Resolve a data plane type.  A data plane type is a string 
// literal that can be resolved to the name of a pipeline module. 
// Examples include:
//
//    - wire
//    - hub
//    - bridge
//    - router
//
// To resolve these names we search the module path for
// a name like "${PATH}/pipeline/{NAME}.so". If we can resolve
// the name, `path` is set to the .so path, and the function
// returns true. Otherwise, this returns false and `path` is
// unmodified.
//
// TODO: Optimize this by rebuilding search strings in a single
// character buffer so we don't keep allocating memory.
bool
resolve_data_plane_type(char const* type, std::string& result) 
{
  for (const std::string& p : cfg.module_path) {
    std::string path = p + "/pipelines";
    std::string mod = path + "/lib" + type + FREEFLOW_EXT_SUFFIX;
    if (!::access(mod.c_str(), F_OK)) {
      result = mod;
      return true;
    }
  }
  return false;
}


} // namespace


// The add data plane command.
//
//    add <dp-name> <dp-type>
//
// Sends the validated command to flowmgr as it is represented 
// in the initial arguments.
//
// FIXME: Do we need to fix the dataplane type so that it
// actually matches a known pipeline? Or should we keep a
// mapping of data plane types to their corresponding
// module paths (this seemms like a more consistent option).
// Where would that mapping come from? SQLite or somehing?
int 
add_dataplane(int argc, char** argv) 
{
  if (!check_argument_arity(argc, 3))
    return -1;
  char const* dp = argv[1];
  char const* type = argv[2];

  std::string path;
  if (!resolve_data_plane_type(type, path)) {
    print(std::cerr, "error: no matching data plane type\n");
    return -1;
  }

  std::string json = json::make_object({
    {"target",  "dp"},
    {"command", "add"},
    {"dp",      dp},
    {"path",    path.c_str()}
  });
  print("sending: {}\n", json);
  return send_command(json);
}


// The delete data plane command.
//
//    del <dp-name>
int 
del_dataplane(int argc, char** argv) 
{ 
  if (!check_argument_arity(argc, 2))
    return -1;
  char const* dp = argv[1];

  std::string json = json::make_object({
    {"target",  "dp"},
    {"command", "del"},
    {"dp",      dp}
  });
  return send_command(json);
}


// Set a dataplane property.
//
//    set <dp-name> up
//    set <dp-name> down
//
// Note that the up/down commands are represented as modify
// commands rather than set. Modify commands change some state
// variable of the target rather than a name/value pair.
int
set_dataplane(int argc, char** argv)
{
  if (!check_argument_arity_at_least(argc, 3))
    return -1;
  char const* dp = argv[1];

  if (argc == 3) {
    char const* state = argv[2];
    
    std::string json = json::make_object({
      {"target",  "dp"},
      {"command", "mod"},
      {"dp",      dp},
      {"value",   state}
    });
    return send_command(json);
  } else {
    char const* prop = argv[2];
    char const* value = argv[3];
    
    std::string json = json::make_object({
      {"target",    "dp"},
      {"command",   "set"},
      {"dp",        dp},
      {"property",  prop},
      {"value",     value}
    });
    return send_command(json);
  }
}


// The show data plane command emits the current configuration
// of the data plane.
//
//    show <dp-name>
//
// Also see the stat command, which shows the running stats for
// the target.
int 
show_dataplane(int argc, char** argv) 
{ 
  if (!check_argument_arity(argc, 2))
    return -1;
  char const* dp = argv[1];

  std::string json = json::make_object({
    {"target",  "dp"},
    {"command", "show"},
    {"dp",      dp}
  });
  return send_command(json);
}


// The stat data plane command.
//
//    stat <dp-name>
//
// Retrieve and display stats for the data plane.
int 
stat_dataplane(int argc, char** argv) 
{ 
  if (!check_argument_arity(argc, 2))
    return -1;
  char const* dp = argv[1];

  std::string json = json::make_object({
    {"target",  "dp"},
    {"command", "stat"},
    {"dp",      dp}
  });
  return send_command(json);
}



// The list data planes command.
//
//    list <dp-name>
int 
list_dataplanes(int argc, char** argv) 
{ 
  if (!check_argument_arity(argc, 1))
    return -1;
  std::string json = json::make_object({
    {"target",  "dp"},
    {"command", "list"}
  });
  return send_command(json);
}


namespace
{

Command_map commands {
  {"add",   add_dataplane},
  {"del",   del_dataplane},
  {"set",   set_dataplane},
  {"show",  show_dataplane},
  {"stat",  stat_dataplane},
  {"list",  list_dataplanes}
};


} // namespace


// Execute a command in the dataplane scope. All such commands
// begin with:
//
//    dp <command-name> [command-args]
int
dataplane_command(int argc, char** argv)
{
  return execute_command(commands, argc, argv);
}

