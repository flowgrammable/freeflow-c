// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWGR_CONTROL_HPP
#define FLOWGR_CONTROL_HPP

// This module defines the communication channel between
// the switch and a control application. 

#include "freeflow/async.hpp"

#include "flowpath/proto.h"

#include <string>

// The server class encapsulates the connection from the control 
// command line tool and is responsible for processing its messages.
// The connection is defined over a UNIX socket specified by
// the path in its constructor.
struct Control_server : ff::Io_handler
{
  Control_server(std::string const&);
  ~Control_server();

  bool on_input() override;
};


// The control channel represents a connected session
// between a control client and this process.
struct Control_channel : ff::Io_handler
{
  Control_channel(int f)
    : Io_handler(f)
  { }

  ~Control_channel();

  bool on_input();
  bool on_reply(fp_reply const*);
};


Control_channel* control_channel();


#endif
