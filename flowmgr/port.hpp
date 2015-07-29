// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWMGR_PORT_HPP
#define FLOWMGR_PORT_HPP

#include "freeflow/async.hpp"
#include "freeflow/json.hpp"

void port_request(ff::Io_handler&, ff::json::Map const&);
void port_reply(ff::Io_handler&, ff::json::Map const&);

#endif