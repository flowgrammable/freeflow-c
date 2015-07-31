// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWCTL_UTIL_HPP
#define FLOWCTL_UTIL_HPP

// This module contains a number of functins used throughout
// the program.

#include "freeflow/format.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>


bool check_argument_arity(int, int);
bool check_argument_arity_at_least(int, int);


// -------------------------------------------------------------------------- //
//                                Commands

// The type of all functions executed by the command line interface.
using Command = int (*)(int, char**);


// Associates a name with a function that defines its meaning.
using Command_map = std::unordered_map<std::string, Command>;


int execute_command(Command_map&, int argc, char**);


// -------------------------------------------------------------------------- //
//                             Communication

int send_command(std::string const&);
int send_command(std::string const&, std::string&);


// -------------------------------------------------------------------------- //
//                             Configuration

// A list of paths.
//
// TODO: Use Boost/std file system?
using Path_list = std::vector<std::string>;


// Stores the runtime configuration of the application.
//
// TODO: Establish reasonable defaults for the search path, etc.
struct Config
{
  // The path to the flowctl socket.
  std::string socket  = "/tmp/flowctl-socket";

  // The path s
  Path_list   module_path;  // Search path for pipeline modules
};


#endif
