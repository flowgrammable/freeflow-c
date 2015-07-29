// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "switch.hpp"
#include "control.hpp"

#include "freeflow/unix.hpp"
#include "freeflow/format.hpp"

#include <iostream>
#include <cerrno>
#include <system_error>

using namespace ff;

// -------------------------------------------------------------------------- //
//                              Server

namespace
{

// The global switch channel.
Switch_channel* switch_;

} // namespace


Switch_server::Switch_server(std::string const& path)
  : Io_handler(ff::server_socket(Unix_socket_address(path)))
{
  if (fd() < 0) {
    throw std::system_error(errno, std::system_category(), "switch server");
  }
  std::cerr << "[nomg] accepting switch connections at '" << path << "'\n";
}


Switch_server::~Switch_server()
{
  close(fd());
}


// Accept a connection. Note that a connection must be pending, 
// otherwise, this will block.
bool
Switch_server::on_input()
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

  // We've already connected a switch instance. Just
  // allow this to terminate.
  if (switch_) {
    std::cerr << "[nomg] switch client already connected\n";
    return false;
  }
  std::cerr << "[nomg] accept switch\n";

  switch_ = new Switch_channel(sd);
  register_handler(switch_);
  return true;
}


// -------------------------------------------------------------------------- //
//                              Channel


// Returns the global switch channel.
Switch_channel*
switch_channel()
{
  return switch_;
}


// Read a message from switch. This should be the response
// to the most recent request.
bool
Switch_channel::on_input()
{
  std::cerr << "[nomg] message from switch\n";

  // Read the message.
  char buf[FP_MESSAGE_LEN];
  int result = recv(fd(), buf, FP_MESSAGE_LEN, 0);
  std::cerr << "[nomg] " << format("read {} bytes from switch\n", result);
  if (result <= 0) {
    if (result < 0)
      std::cerr << std::strerror(errno) << '\n';
    return false;
  }

  Control_channel* noctl = control_channel();
  noctl->on_reply((fp_reply*)buf);
  return true;
}


Switch_channel::~Switch_channel()
{
  close(fd());
  switch_ = nullptr;

  // If this is going down, be sure to take down a
  // connected noctl client, so we don't leave it hanging.
  if (Control_channel* noctl = control_channel()) {
    send(noctl->fd(), "error: switch shutdown unexpectedly");
    close(noctl->fd());
  }
}


// Send the request to the switch instance. Returns false
// if an error occurred while sending the message.
bool
Switch_channel::on_request(fp_request const* req)
{
  std::cerr << "[nomg] sending message to switch\n";
  int result = send(fd(), req, FP_MESSAGE_LEN, 0);  
  if (result < 0) {
    std::cerr << std::strerror(errno) << '\n';
    return false;
  }
  return true;
}
