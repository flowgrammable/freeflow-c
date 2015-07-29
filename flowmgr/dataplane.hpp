// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWGR_DATAPLANE_HPP
#define FLOWGR_DATAPLANE_HPP

#include "freeflow/async.hpp"
#include "freeflow/json.hpp"

void dataplane_request(ff::Io_handler&, ff::json::Map const&);
void dataplane_reply(ff::Io_handler&, ff::json::Map const&);

#endif
