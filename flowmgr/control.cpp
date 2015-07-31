// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "control.hpp"
#include "request.hpp"
#include "utility.hpp"

#include "freeflow/unix.hpp"
#include "freeflow/json.hpp"
#include "freeflow/memory.hpp"
#include "freeflow/format.hpp"

#include "flowpath/error.h"

#include <cerrno>
#include <iostream>
#include <system_error>

#include <unistd.h>


using namespace ff;

// -------------------------------------------------------------------------- //
//                              Server

namespace
{

// The global flowctl channel.
Control_channel* ctrl_;

} // namespace


Control_server::Control_server(std::string const& path)
  : Io_handler(ff::server_socket(Unix_socket_address(path)))
{
  if (fd() < 0)
    throw std::system_error(errno, std::system_category(), "flowctl server");

  std::cout << "[flowmgr] accepting flowctl connections at '" << path << "'\n";
}


Control_server::~Control_server()
{
  close(fd());
}


// Accept a connection when one is available.
bool
Control_server::on_input()
{
  extern void register_handler(Io_handler*);
  
  // Accept the connection and buil a new I/O handler.
  //
  // TODO: Improve error handling.
  int sd =  ff::accept(fd());
  if (sd < 0) {
    std::cerr << std::strerror(errno) << '\n';
    return false;
  }

  // If there's a client already connected, shutdown
  // second connection.
  //
  // TODO: Accept multiple connections
  if (ctrl_) {
    std::cerr << "[flowmgr] flowctl already accepted\n";
    return false;
  }
  std::cerr << "[flowmgr] accepting flowctl client\n";

  // Create the handler for this flowctl channel and
  // register it.
  ctrl_ = new Control_channel(sd);
  register_handler(ctrl_);

  // Require that flowpath is running.
  require_switch(*ctrl_);

  return true;
}


// -------------------------------------------------------------------------- //
//                              Channel

// Returns the current flowctl channel.
Control_channel*
control_channel()
{
  return ctrl_;
}


Control_channel::~Control_channel()
{
  std::cout << "[flowmgr] closing flowctl\n";
  close(fd());
  ctrl_ = nullptr;
}


// Read and execute the command sent by flowctl.
bool
Control_channel::on_input()
{
  constexpr int bufsize = 1024;
  char buf[bufsize];

  // Read the message.
  int nbytes = recv(fd(), buf);
  if (nbytes <= 0) {
    if (nbytes < 0) {
      std::cerr << std::strerror(errno) << '\n';
    }
    return false;    
  }
  
  // FIXME: This is pretty dumb. We should probably buffer
  // this and then process the full command.
  if (nbytes == bufsize) {
    std::cerr << "error: message too large\n";
    return false;
  }

  // Interpret the message.
  return request(*this, buf, nbytes);
}


// Send the result to the caller.
//
// FIXME: A lot of operations return more than just success/fail. For
// example, if we get the dataplane state, then we should return 
// that information to the caller.
bool
Control_channel::on_reply(fp_reply const* rep)
{
  // FIXME: This is broken.
  std::string result;
  if (rep->result == FP_SUCCESS)
    result = "ok";
  else
    result = std::string("error: ") + fp_strerror(rep->result);

  // TODO: Diagnose system errors here.  
  return send(fd(), result.c_str(), result.size()) >= 0;
}

