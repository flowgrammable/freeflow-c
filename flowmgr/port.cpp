// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "dataplane.hpp"
#include "dispatch.hpp"
#include "control.hpp"
#include "switch.hpp"
#include "utility.hpp"

#include "freeflow/format.hpp"

#include "flowpath/proto.h"
#include "flowpath/error.h"

#include <iostream>


using namespace ff;

namespace
{


void
add_request(Io_handler& io, json::Map const& map)
{
  json::String const* name = get_string_arg(map, "dp");
  if (!name)
    send_error(io, "missing name");
  json::String const* type = get_string_arg(map, "type");
  if (!name)
    send_error(io, "missing type");
  json::String const* device = get_string_arg(map, "device");
  if (!name)
    send_error(io, "missing device");
  json::String const* options = get_string_arg(map, "options");
  if (!name)
    send_error(io, "missing options");

  // Construct a message and send it.
  char buf[FP_MESSAGE_LEN];
  fp_request* req = fp_make_request(buf);
  auto* args = fp_get_port_add_arguments(req);
  std::copy(name->begin(), name->end(), args->name);
  std::copy(type->begin(), type->end(), args->type);
  std::copy(device->begin(), device->end(), args->device);
  std::copy(options->begin(), options->end(), args->options);
  bool res = switch_channel()->on_request(req);
  if (!res)
    send_error(io, "request failed");
}


void
del_request(Io_handler& io, json::Map const& map)
{
  json::String const* name = get_string_arg(map, "dp");
  if (!name)
    send_error(io, "missing name");

  json::String const* pid = get_string_arg(map, "pid");
  if (!name)
    send_error(io, "missing port id");


  // Construct a message and send it.
  char buf[FP_MESSAGE_LEN];
  fp_request* req = fp_make_request(buf);
  auto* args = fp_get_port_del_arguments(req);
  std::copy(name->begin(), name->end(), args->name);
  std::copy(pid->begin(), pid->end(), args->pid);
  bool res = switch_channel()->on_request(req);
  if (!res)
    send_error(io, "request failed");
}


void
list_request(Io_handler& io, json::Map const& map)
{
  json::String const* name = get_string_arg(map, "dp");
  if (!name)
    send_error(io, "missing name");
  char buf[FP_MESSAGE_LEN];
  fp_request* req = fp_make_request(buf);
  auto* args = fp_get_port_list_arguments(req);
  std::copy(name->begin(), name->end(), args->name);
  bool res = switch_channel()->on_request(req);
  if (!res)
    send_error(io, "request failed");
}


Dispatch_table reqs_ {
  {"add",  add_request},
  {"del",  del_request},
  {"list", list_request}
};

} // end namespace

void
port_request(Io_handler& io, json::Map const& args)
{
  json::String const* cmd = get_string_arg(args, "command");
  if (!cmd) {
    send_error(io, "missing port command");
    return;
  }
  auto iter = reqs_.find(cmd->str());
  if (iter == reqs_.end()) {
    send_error(io, "unknown port command");
    return;
  }
  return iter->second(io, args);
}

void
port_reply(Io_handler& io, json::Map const& args)
{
  
}