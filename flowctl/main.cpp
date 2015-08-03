// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "util.hpp"
#include "dataplane.hpp"
#include "port.hpp"

#include <iostream>
#include <unordered_map>


using namespace ff;


// The global configuration.
Config cfg;



// -------------------------------------------------------------------------- //
//                           Basic commands


// Ping the flowmgr server and wait for a result.
//
//    ping
//
// The server will respond with its version and
// configuration.
int
ping_command(int, char**)
{
  std::string result;
  send_command("{\"command\":\"ping\"}", result);
  return 0;
}


// -------------------------------------------------------------------------- //
//                           Program information

// TODO: Print a list of help topics and commands...
void
help()
{
  print("help topics....\n");
}


// TODO: Lookup and print help on the given topic.
void
help(char const* topic)
{
  print("help on '{}'\n", topic);
}


// TODO: Print a list of help topics and commands...
int
help_command(int argc, char** argv)
{
  if (argc == 0)
    help();
  else
    help(argv[1]);
  return 0;
}


// TODO: Make the version some kind of compile-time constant
// that is provided by the build system.
//
// TODO: Other information (legal?).
int
version_command(int, char**)
{
  print("nocl 0.1\n");
  print("Copyright (c) 2014-2015 Flowgrammable.org\n");
  print("All rights reserved\n");
  return 0;
}



// -------------------------------------------------------------------------- //
//                         Command line options
//
// TODO: Actually find and load a configuration file in order
// to define reasonable defaults for the application.
//
// FIXME: Factor CLI and ENV and other configuration facilities into
// a single application configuration facility.

namespace
{

// Parse a search path. This is a colon-delimited list of 
// path names.
//
// TODO: This retains empty path segments. It might be nice
// to strip those... maybe not.
Path_list
parse_search_path(std::string const& str)
{
  Path_list ret;
  std::size_t n = 0;
  while (n != std::string::npos) {
    std::size_t m = str.find_first_of(':', n);
    if (m == std::string::npos)
      break;
    ret.push_back(str.substr(n, m - n));
    n = m + 1;
  }
  ret.push_back(str.substr(n));
  return ret;
}

} // namespace


// Parse options from the environment. Returns true if there
// are no errors, false otherwise.
bool
parse_environment_options()
{
  if (char const* path = std::getenv("FREEFLOW_MODULE_PATH"))
    cfg.module_path = parse_search_path(path);
  return true;
}


// -------------------------------------------------------------------------- //
//                                   Main


namespace
{

// Available commands.
//
// TODO: We should actually be loading DLLs to extend the set
// of commands for specific applications. For example, we 
// might enable `flowctl br run dp1` or `flowctl route add ...`
// if these applications are installed on the switch.
Command_map commands {
  {"help",    help_command},
  {"version", version_command},
  {"ping",    ping_command},

  {"dp",      dataplane_command},
  {"port",    port_command},
  // {"table",   table_command},
  // {"meter",   meter_command},
  // {"group",   group_command},
};


} // namespace


int 
main(int argc, char** argv)
{
  if (!parse_environment_options())
    return -1;

  if (argc == 1) {
    print(std::cerr, "error: no commands given\n");
    print(std::cerr, "type 'flowctl help' for a list of commands\n");
    return -1;
  }

  auto iter = commands.find(argv[1]);
  if (iter == commands.end()) {
    print(std::cerr, "error: no command named '{}'\n", argv[1]);
    print(std::cerr, "type 'flowctl help' for a list of commands\n");
    return -1;
  }
  Command cmd = iter->second;

  // Run the command.
  return cmd(argc - 1, argv + 1);
}
