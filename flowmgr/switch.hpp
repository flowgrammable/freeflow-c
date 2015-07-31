// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWGR_SWITCH_HPP
#define FLOWGR_SWITCH_HPP

// This module defines the communication channel between
// flowmgr and the switch. 

#include "freeflow/async.hpp"

#include "flowpath/proto.h"

#include <string>


// The channel class encapsulates the connection from
// the flowctl command line tool and is responsible for
// processing its messages. 
struct Switch_server : ff::Io_handler
{
  Switch_server(std::string const&);
  ~Switch_server();

  bool on_input();
};


// The switch channel represents a connected session
// between a switch client and this process.
//
// TODO: How many switch channels should we support?
// Initially, we should have only one switch service
// attached to a nomg.
struct Switch_channel : ff::Io_handler
{
  Switch_channel(int f)
    : Io_handler(f)
  { }

  ~Switch_channel();

  bool on_input();
  bool on_request(fp_request const*);
};


Switch_channel* switch_channel();


#endif
