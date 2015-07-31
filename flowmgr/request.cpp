// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "request.hpp"
#include "switch.hpp"
#include "dispatch.hpp"
#include "dataplane.hpp"
#include "port.hpp"
#include "utility.hpp"

#include "freeflow/json.hpp"


using namespace ff;
using json::as;

namespace
{


// On receiving the ping command, respond with the message 'pong'.
bool
on_ping(Io_handler& ctrl, std::stringstream&)
{
  return send(ctrl.fd(), "pong");
}


void
process_target(Io_handler& io, json::Map const& args, std::string const& tgt)
{
  // FIXME: This should be done on startup.
  //
  // TODO: How do we make this extensible? We should be able
  // to load modules an startup and build this dispatch table
  // dynamically.
  static Dispatch_table dispatch;
  if (dispatch.empty()) {
    Dispatch_table tmp {
      {"dp",   dataplane_request},
      {"port", port_request}
      // {"table", table_request},
      // {"meter", meter_request}
      // {"group", group_request}
    };
    dispatch.swap(tmp);
  }

  // Lookup and dispatch the command.
  auto iter = dispatch.find(tgt);
  if (iter == dispatch.end()) {
    send_error(io, "unsupported command");
    return;
  }
  iter->second(io, args);
}


void
process(Io_handler& io, json::Map const& args)
{
  if (json::Value* tgt = args["target"])
    if (json::String* str = as<json::String>(tgt))
      return process_target(io, args, str->str());

  // FIXME: Do we support non-targeted commands?
  send_error(io, "ill-formed command");
}


// Initial processng of the JSON request. This ensures that
// the outer-most value is an object before forwarding the
// command to the lookup table.
void
process(Io_handler& io, json::Value const* json)
{
  if (json::Object const* obj = as<json::Object>(json))
    process(io, obj->map());
  else
    send_error(io, "ill-formed command");
}


} // namespace


bool
request(Io_handler& io, char const* buf, int len)
{
  // A (very) small 4K buffer for JSON parsing. Our JSON input should
  // typically be very, very smal -- no more than 100 objects. This
  // should be just large enough to hold that.
  //
  // TODO: We could actually switch to a sequent allocator if we
  // see that the string is larger than say 2K.
  constexpr int limit = 4096;
  static char arena[limit];

  // Parse the command.
  Buffer_allocator alloc(arena, limit);
  json::Document doc(alloc);
  json::Value* json = nullptr;
  try {
    json =  doc.parse(buf, buf + len);
  } catch (std::exception& e) {
    send_error(io, e.what());
  }

  print("[flowmgr] got request: {}\n", *json);

  // Handle the command.
  if (json)
    process(io, json);
  else
    send_error(io, "empty request");
  
  return true;
}

