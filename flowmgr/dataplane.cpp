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

  json::String const* type = get_string_arg(map, "path");
  if (!type)
    return send_error(io, "missing type");

  // Construct and send the message.
  char buf[FP_MESSAGE_LEN];
  fp_request* req = fp_make_request(buf);
  auto* args = fp_get_dataplane_add_arguments(req);
  std::copy(name->begin(), name->end(), args->name);
  std::copy(type->begin(), type->end(), args->type);
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

  // Construct and send the message.
  char buf[FP_MESSAGE_LEN];
  fp_request* req = fp_make_request(buf);
  auto* args = fp_get_dataplane_del_arguments(req);
  std::copy(name->begin(), name->end(), args->name);
  switch_channel()->on_request(req);
}


void
set_request(Io_handler& io, json::Map const& map)
{
  // char buf[FP_MESSAGE_LEN];
  // fp_request* req = fp_set_dataplane_state(buf, state);
  // return noproto_channel()->on_request(req);
}


void
show_request(Io_handler& io, json::Map const& map)
{
}


void
stat_request(Io_handler& io, json::Map const& map)
{
}


void
list_request(Io_handler& io, json::Map const& map)
{

}


Dispatch_table reqs_ {
  {"add",  add_request},
  {"del",  del_request},
  {"set",  set_request},
  {"show", show_request},
  {"stat", stat_request},
  {"list", list_request}
};


} // namespace


void 
dataplane_request(Io_handler& io, json::Map const& args)
{
  json::String const* cmd = get_string_arg(args, "command");
  if (!cmd) {
    send_error(io, "missing dataplane command");
    return;
  }
  auto iter = reqs_.find(cmd->str());
  if (iter == reqs_.end()) {
    send_error(io, "unknown dataplane command");
    return;
  }
  return iter->second(io, args);
}


void 
dataplane_reply(Io_handler& io, json::Map const& args)
{

}
