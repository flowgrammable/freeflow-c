// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#include "utility.hpp"
#include "switch.hpp"
#include "control.hpp"

using namespace ff;

// Get the connected noproto channel so we can forward commands. 
// If noproto is not running, emit the error  message to noctl and 
// return nulltpr.
Switch_channel*
require_switch(Io_handler& io)
{
  if (Switch_channel* ch = switch_channel())
    return ch;
  send_error(io, "noproto not running");
  return nullptr;
}

