// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWGR_REQUEST_HPP
#define FLOWGR_REQUEST_HPP

#include "freeflow/async.hpp"

// This module is responsible for the handling of requests from
// maanagement clients. Every request is analyzed and forwarded
// to a flowpath instance.
//
// See the reply module for the framwork for handling responses
// from flowpath.

bool request(ff::Io_handler&, char const*, int);

#endif
