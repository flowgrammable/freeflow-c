// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWMGR_UTILITY_HPP
#define FLOWMGR_UTILITY_HPP

// A collection of useful facilities.

#include "freeflow/async.hpp"
#include "freeflow/socket.hpp"
#include "freeflow/json.hpp"
#include "freeflow/format.hpp"


struct Control_channel;
struct Switch_channel;


// Returns a string-valued argument or nullptr if the argument
// is either not in the argument map or not a string.
inline ff::json::String const*
get_string_arg(ff::json::Map const& args, char const* arg)
{
  using ff::json::as;
  auto iter = args.find(arg);
  if (iter != args.end())
    return as<ff::json::String>(iter->second);
  else
    return nullptr;
}


// Send a response to the client.
template<typename... Args>
inline void
send(ff::Io_handler& io, char const* msg, Args const&... args)
{
  ff::send(io.fd(), ff::format(msg, args...));
}


// Send a JSON-formatted error message to the IO handler.
inline void
send_error(ff::Io_handler& io, char const* msg)
{
  std::string json = ff::json::make_object({
    {"status", "error"},
    {"messsage", msg}
  });
  ff::send(io.fd(), json);
}


// Send a JSON-formatted error message to the IO handler.
inline void
send_error(ff::Io_handler& io, std::string const& msg)
{
  send_error(io, msg.c_str());
}


Control_channel* require_control(ff::Io_handler&);
Switch_channel*  require_switch(ff::Io_handler&);


#endif
